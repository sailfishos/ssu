/**
 * @file ssuvariables.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include <QStringList>
#include <QRegExp>
#include <QStringRef>

#include "ssuvariables.h"

#include "../constants.h"

SsuVariables::SsuVariables(): QObject() {

}

void SsuVariables::resolveSection(QSettings *settings, QString section, QHash<QString, QString> *storageHash){
  QStringList repoVariables;

  settings->beginGroup(section);
  repoVariables = settings->allKeys();
  foreach (const QString &key, repoVariables){
    storageHash->insert(key, settings->value(key).toString());
  }
  settings->endGroup();
}

QString SsuVariables::resolveString(QString pattern, QHash<QString, QString> *variables, int recursionDepth){
  if (recursionDepth >= SSU_MAX_RECURSION){
    return "maximum-recursion-level-reached";
  }

  QRegExp regex("%\\\([^%]*\\\)", Qt::CaseSensitive, QRegExp::RegExp2);
  regex.setMinimal(true);

  int pos = 0;
  while ((pos = regex.indexIn(pattern, pos)) != -1){
    QString match = regex.cap(0);

    if (match.contains(":")){
      // variable is special, resolve before replacing
      QString variable = resolveVariable(match, variables);
      pattern.replace(match, variable);
      pos += variable.length();
    } else {
      // look up variable name in hashmap, and replace it with stored value,
      // if found, or ""
      QString variableName = match;
      variableName.remove(0,2);
      variableName.chop(1);
      if (variables->contains(variableName)){
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

QString SsuVariables::resolveVariable(QString variable, QHash<QString, QString> *variables){
  QString variableValue = "";

  if (variable.endsWith(")"))
    variable.chop(1);
  if (variable.startsWith("%("))
    variable.remove(0,2);

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
  if (variable.length() > magic +1)
    op = variable.at(magic + 1);

  switch(op.toAscii()){
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
