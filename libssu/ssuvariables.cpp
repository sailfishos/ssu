/**
 * @file ssuvariables.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include <QStringList>

#include "ssuvariables.h"

// TODO: Add a simple variable parser to allow variable substitution; it should
//       get called in the resolve sections
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
