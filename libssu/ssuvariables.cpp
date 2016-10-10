/**
 * @file ssuvariables.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include <QStringList>
#include <QRegExp>
#include <QStringRef>

#include "ssuvariables_p.h"
#include "ssulog_p.h"

#include "../constants.h"

SsuVariables::SsuVariables(): QObject()
{

}

QString SsuVariables::defaultSection(SsuSettings *settings, QString section)
{
    QStringList parts = section.split("-");

    if (section.startsWith("var-"))
        parts.insert(1, "default");
    else
        parts.replace(0, "default");

    QString key = parts.join("-");

    if (settings->childGroups().contains(key))
        return key;
    else
        return "";
}

QString SsuVariables::resolveString(QString pattern, QHash<QString, QString> *variables, int recursionDepth)
{
    if (recursionDepth >= SSU_MAX_RECURSION) {
        return "maximum-recursion-level-reached";
    }

    QRegExp regex("%\\([^%]*\\)", Qt::CaseSensitive, QRegExp::RegExp2);
    regex.setMinimal(true);

    int pos = 0;
    while ((pos = regex.indexIn(pattern, pos)) != -1) {
        QString match = regex.cap(0);

        if (match.contains(":")) {
            // variable is special, resolve before replacing
            QString variable = resolveVariable(match, variables);
            pattern.replace(match, variable);
            pos += variable.length();
        } else {
            // look up variable name in hashmap, and replace it with stored value,
            // if found, or ""
            QString variableName = match;
            variableName.remove(0, 2);
            variableName.chop(1);
            if (variables->contains(variableName)) {
                pattern.replace(match, variables->value(variableName));
                pos += variables->value(variableName).length();
            } else
                pattern.replace(match, "");
        }
    }

    // check if string still contains variables, and recurse
    if (regex.indexIn(pattern, 0) != -1)
        pattern = resolveString(pattern, variables, recursionDepth + 1);

    return pattern;
}

QString SsuVariables::resolveVariable(QString variable, QHash<QString, QString> *variables)
{
    QString variableValue = "";

    if (variable.endsWith(")"))
        variable.chop(1);
    if (variable.startsWith("%("))
        variable.remove(0, 2);

    // hunt for your colon
    int magic = variable.indexOf(":");

    // seems you misplaced your colon
    if (magic == -1) return variable;

    QStringRef variableName(&variable, 0, magic);
    QStringRef variableSub(&variable, magic + 2, variable.length() - magic - 2);

    // Fill in variable value for later tests, if it exists
    if (variables->contains(variableName.toString()))
        variableValue = variables->value(variableName.toString());

    // find the operator who's after your colon
    QChar op;
    if (variable.length() > magic + 1)
        op = variable.at(magic + 1);

    switch (op.toLatin1()) {
    case '-':
        // substitute default value if variable is empty
        if (variableValue == "")
            return variableSub.toString();
        break;
    case '+':
        // substitute default value if variable is not empty
        if (variableValue != "")
            return variableSub.toString();
        break;
    case '=': {
        // %(%(foo):=bar?foobar|baz)
        // if foo == bar then return foobar, else baz
        QString sub = variableSub.toString();
        QString a = sub.left(sub.indexOf("?"));
        QString b = sub.right(sub.length() - sub.indexOf("?") - 1);
        if (b.indexOf("|") == -1)
            return b;
        if (variableName == a)
            return b.left(b.indexOf("|"));
        else
            return b.right(b.length() - b.indexOf("|") - 1);
    }
    }

    // no proper substitution found -> return default value
    return variableValue;
}

void SsuVariables::setSettings(SsuSettings *settings)
{
    m_settings = settings;
}

SsuSettings *SsuVariables::settings()
{
    return m_settings;
}

/// @todo add override capability with an override-section in ssu.ini
QVariant SsuVariables::variable(QString section, const QString &key)
{
    if (m_settings != NULL)
        return variable(m_settings, section, key);
    else
        return QVariant();
}

QVariant SsuVariables::variable(SsuSettings *settings, QString section, const QString &key)
{
    QVariant value;

    value = readVariable(settings, section, key, 0);

    // first check if the value is defined in the main section, and fall back
    // to default sections
    if (value.type() == QMetaType::UnknownType) {
        QString dSection = defaultSection(settings, section);
        if (!dSection.isEmpty())
            value = readVariable(settings, dSection, key, 0, false);
    }

    return value;
}

void SsuVariables::variableSection(QString section, QHash<QString, QString> *storageHash)
{
    if (m_settings != NULL)
        variableSection(m_settings, section, storageHash);
}

void SsuVariables::variableSection(SsuSettings *settings, QString section, QHash<QString, QString> *storageHash)
{

    QString dSection = defaultSection(settings, section);
    if (dSection.isEmpty())
        readSection(settings, section, storageHash, 0);
    else {
        readSection(settings, dSection, storageHash, 0);
        readSection(settings, section, storageHash, 0, false);
    }
}

// resolve a configuration section, recursively following all 'variables' sections.
// variables which exist in more than one section will get overwritten when discovered
// again
// the section itself gets evaluated at the end, again having a chance to overwrite variables
void SsuVariables::readSection(SsuSettings *settings, QString section,
                               QHash<QString, QString> *storageHash, int recursionDepth,
                               bool logOverride)
{
    if (recursionDepth >= SSU_MAX_RECURSION) {
        SsuLog::instance()->print(LOG_WARNING,
                                  QString("Maximum recursion depth for resolving section %1 from %2")
                                  .arg(section)
                                  .arg(settings->fileName()));
        return;
    }

    if (settings->contains(section + "/variables")) {
        // child should log unless the parent is a default section
        bool childLogOverride = true;
        if (section.startsWith("default-") || section.startsWith("var-default-"))
            childLogOverride = false;

        QStringList sections = settings->value(section + "/variables").toStringList();
        foreach (const QString &section, sections) {
            if (section.startsWith("var-"))
                readSection(settings, section, storageHash, recursionDepth + 1, childLogOverride);
            else
                readSection(settings, "var-" + section, storageHash,
                            recursionDepth + 1, childLogOverride);
        }
    }

    settings->beginGroup(section);
    if (settings->group() != section)
        return;

    QStringList locals;
    if (settings->contains("local"))
        locals = settings->value("local").toStringList();

    QStringList keys = settings->allKeys();
    foreach (const QString &key, keys) {
        // local variable
        if (key.startsWith("_"))
            continue;

        if (locals.contains(key))
            continue;

        if (key == "variables" || key == "local")
            continue;

        if (storageHash->contains(key) && logOverride) {
            SsuLog::instance()->print(LOG_DEBUG,
                                      QString("Variable %1 overwritten from %2::%3")
                                      .arg(key)
                                      .arg(settings->fileName())
                                      .arg(section));
        }
        storageHash->insert(key, settings->value(key).toString());
    }
    settings->endGroup();
}

QVariant SsuVariables::readVariable(SsuSettings *settings, QString section, const QString &key,
                                    int recursionDepth, bool logOverride)
{
    Q_UNUSED(logOverride)

    QVariant value;

    if (recursionDepth >= SSU_MAX_RECURSION) {
        SsuLog::instance()->print(LOG_WARNING,
                                  QString("Maximum recursion depth for resolving %1 from %2::%3")
                                  .arg(key)
                                  .arg(settings->fileName())
                                  .arg(section));
        return value;
    }

    // variables directly in the section take precedence
    if (settings->contains(section + "/" + key)) {
        return settings->value(section + "/" + key);
    }

    /// @todo add logging for overrides
    if (settings->contains(section + "/variables")) {
        // child should log unless the parent is a default section
        bool childLogOverride = true;
        if (section.startsWith("default-") || section.startsWith("var-default-"))
            childLogOverride = false;

        QStringList sections = settings->value(section + "/variables").toStringList();
        foreach (const QString &section, sections) {
            if (section.startsWith("var-"))
                value = readVariable(settings, section, key, recursionDepth + 1, childLogOverride);
            else
                value = readVariable(settings, "var-" + section, key,
                                     recursionDepth + 1, childLogOverride);
        }
    }

    return value;
}
