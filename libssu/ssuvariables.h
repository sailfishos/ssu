/**
 * @file ssuvariables.h
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#ifndef _SSUVARIABLES_H
#define _SSUVARIABLES_H

#include <QObject>
#include <QHash>

#include "ssusettings.h"

class SsuVariables: public QObject {
    Q_OBJECT

  public:
    SsuVariables();
    /**
     * Look up all variables in the specified configuration file section,
     * run them through the variable expander, and add them to the supplied
     * QHash
     */
    void resolveSection(QString section, QHash<QString, QString> *storageHash);
    static void resolveSection(SsuSettings *settings, QString section, QHash<QString, QString> *storageHash);
    /**
     * Resolve a whole string, containing several variables. Variables inside variables are allowed
     */
    static QString resolveString(QString pattern, QHash<QString, QString> *variables, int recursionDepth=0);
    /**
     * Resolve variables; variable can be passed as %(var) or var
     */
    static QString resolveVariable(QString variable, QHash<QString, QString> *variables);
    /**
     * Set the settings object to use
     */
    void setSettings(SsuSettings *settings);
    /*
     * Return the settings object used
     */
    SsuSettings *settings();
    /**
     * Return a variable from the given variable section. 'var'- is automatically
     * prepended to the section name if not specified already. Recursive search
     * through several variable sections (specified in the section) is supported,
     * returned will be the first occurence of the variable.
     */
    QVariant variable(QString section, const QString &key);
    static QVariant variable(SsuSettings *settings, QString section, const QString &key);
    /**
     * Return the requested variable section. 'var-' is automatically
     * prepended to the section name if not specified already.
     */
    void variableSection(QString section, QHash<QString, QString> *storageHash);
    static void variableSection(SsuSettings *settings, QString section, QHash<QString, QString> *storageHash);

  private:
    SsuSettings *m_settings;
};

#endif
