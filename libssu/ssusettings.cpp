/**
 * @file ssusettings.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include <QStringList>
#include <QDirIterator>
#include <QFileInfo>
#include <QDateTime>

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

SsuSettings::SsuSettings(const QString &fileName, const QString &settingsDirectory, QObject *parent):
  QSettings(fileName, QSettings::IniFormat, parent){
  settingsd = settingsDirectory;
  merge();
}

void SsuSettings::merge(bool keepOld){
  if (settingsd == "")
    return;

  bool skipMerge = true;

  SsuLog *ssuLog = SsuLog::instance();

  QDirIterator it(settingsd, QDir::AllEntries|QDir::NoDot|QDir::NoDotDot, QDirIterator::FollowSymlinks);
  QStringList settingsFiles;

  QFileInfo oldSettingsInfo(fileName());

  while (it.hasNext()){
    QString f = it.next();

    settingsFiles.append(it.filePath());

    QFileInfo info(it.filePath());
    if (info.lastModified() >= oldSettingsInfo.lastModified())
      skipMerge = false;
  }

  if (skipMerge){
    ssuLog->print(LOG_DEBUG, QString("Configuration file is newer than all config.d files, skipping merge"));
    return;
  }

  settingsFiles.sort();

  // delete all keys in the cached settings
  if (settingsFiles.count() > 0 && !keepOld)
    clear();

  merge(this, settingsFiles);
  sync();
}

void SsuSettings::merge(QSettings *masterSettings, const QStringList &settingsFiles){
  SsuLog *ssuLog = SsuLog::instance();

  foreach (const QString &settingsFile, settingsFiles){
    QSettings settings(settingsFile, QSettings::IniFormat);
    QStringList groups = settings.childGroups();

    ssuLog->print(LOG_DEBUG, QString("Merging %1 into %2")
                  .arg(settingsFile)
                  .arg(masterSettings->fileName()));

    foreach (const QString &group, groups){
      masterSettings->beginGroup(group);
      settings.beginGroup(group);

      QStringList keys = settings.allKeys();
      foreach (const QString &key, keys){
        masterSettings->setValue(key, settings.value(key));
      }

      settings.endGroup();
      masterSettings->endGroup();
    }
  }
}

/*
 * If you change anything here, run `make update-upgrade-test-recipe` inside
 * tests/ut_settings/ and check the impact of your changes with
 * `git diff testdata/upgrade/recipe`. See ut_settings/upgradetesthelper.cpp for
 * more details.
 */
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
    sync();
  }
}
