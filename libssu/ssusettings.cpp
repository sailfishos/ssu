/**
 * @file ssusettings.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include <QStringList>
#include <QDirIterator>

#include "ssusettings.h"
#include "ssulog.h"

SsuSettings::SsuSettings(): QSettings(){

}

SsuSettings::SsuSettings(const QString &fileName, Format format, QObject *parent):
  QSettings(fileName, format, parent){
}

SsuSettings::SsuSettings(const QString &fileName, Format format, const QString &defaultFileName, QObject *parent):
  QSettings(fileName, format, parent){
  defaultSettingsFile = defaultFileName;
  upgrade();
}

void SsuSettings::merge(){
  if (settingsd == "")
    return;

  QDirIterator it(settingsd, QDirIterator::FollowSymlinks);
  // TODO: only rewrite the settings file if something has changed
  while (it.hasNext()){
    QString f = it.next();
    QSettings settings(f, QSettings::IniFormat);
    QStringList groups = settings.childGroups();
    foreach (const QString &group, groups){
      beginGroup(group);
      settings.beginGroup(group);

      QStringList keys = settings.allKeys();
      foreach (const QString &key, keys){
        setValue(key, settings.value(key));
      }

      settings.endGroup();
      endGroup();
      sync();
    }
  }
}

void SsuSettings::upgrade(){
  int configVersion=0;
  int defaultConfigVersion=0;

  SsuLog *ssuLog = SsuLog::instance();

  if (defaultSettingsFile == "")
    return;

  QSettings defaultSettings(defaultSettingsFile, QSettings::IniFormat);

  if (contains("configVersion"))
    configVersion = value("configVersion").toInt();
  if (defaultSettings.contains("configVersion"))
    defaultConfigVersion = defaultSettings.value("configVersion").toInt();

  if (configVersion < defaultConfigVersion){
    ssuLog->print(LOG_DEBUG, QString("Configuration is outdated, updating from %1 to %2")
                 .arg(configVersion)
                 .arg(defaultConfigVersion));

    for (int i=configVersion+1;i<=defaultConfigVersion;i++){
      QStringList defaultKeys;
      QString currentSection = QString("%1/").arg(i);

      ssuLog->print(LOG_DEBUG, QString("Processing configuration version %1").arg(i));
      defaultSettings.beginGroup(currentSection);
      defaultKeys = defaultSettings.allKeys();
      defaultSettings.endGroup();
      foreach (const QString &key, defaultKeys){
        // Default keys support both commands and new keys
        if (key.compare("cmd-remove", Qt::CaseSensitive) == 0){
          // Remove keys listed in value as string list
          QStringList oldKeys = defaultSettings.value(currentSection + key).toStringList();
          foreach (const QString &oldKey, oldKeys){
            if (contains(oldKey)){
              remove(oldKey);
              ssuLog->print(LOG_DEBUG, QString("Removing old key: %1").arg(oldKey));
            }
          }
        } else if (!contains(key)){
          // Add new keys..
          setValue(key, defaultSettings.value(currentSection + key));
          ssuLog->print(LOG_DEBUG, QString("Adding key: %1").arg(key));
        } else {
          // ... or update the ones where default values has changed.
          QVariant oldValue;

          // check if an old value exists in an older configuration version
          for (int j=i-1;j>0;j--){
            if (defaultSettings.contains(QString("%1/").arg(j)+key)){
              oldValue = defaultSettings.value(QString("%1/").arg(j)+key);
              break;
            }
          }

          // skip updating if there is no old value, since we can't check if the
          // default value has changed
          if (oldValue.isNull())
            continue;

          QVariant newValue = defaultSettings.value(currentSection + key);
          if (oldValue == newValue){
            // old and new value match, no need to do anything, apart from beating the
            // person who added a useless key
            continue;
          } else {
            // default value has changed, so check if the configuration is still
            // using the old default value...
            QVariant currentValue = value(key);
            // testcase: handles properly default update of thing with changed value in ssu.ini?
            if (currentValue == oldValue){
              // ...and update the key if it does
              setValue(key, newValue);
              ssuLog->print(LOG_DEBUG, QString("Updating %1 from %2 to %3")
                           .arg(key)
                           .arg(currentValue.toString())
                           .arg(newValue.toString()));
            }
          }
        }
      }
      setValue("configVersion", i);
    }
  }
}
