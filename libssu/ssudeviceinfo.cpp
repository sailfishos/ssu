/**
 * @file ssudeviceinfo.cpp
 * @copyright 2013 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include <QSystemDeviceInfo>
#include <QTextStream>
#include <QDir>

#include "ssudeviceinfo.h"
#include "ssucoreconfig.h"
#include "ssulog.h"

#include "../constants.h"

QTM_USE_NAMESPACE

SsuDeviceInfo::SsuDeviceInfo(QString model): QObject(){

    boardMappings = new SsuSettings(SSU_BOARD_MAPPING_CONFIGURATION, SSU_BOARD_MAPPING_CONFIGURATION_DIR);
    if (!model.isEmpty())
      cachedModel = model;
}

QStringList SsuDeviceInfo::adaptationRepos(){
  QStringList result;

  QString model = deviceVariant(true);

  if (boardMappings->contains(model + "/adaptation-repos"))
    result = boardMappings->value(model + "/adaptation-repos").toStringList();

  return result;
}

QString SsuDeviceInfo::adaptationVariables(const QString &adaptationName, QHash<QString, QString> *storageHash){
  SsuLog *ssuLog = SsuLog::instance();
  QStringList adaptationRepoList = adaptationRepos();
  // special handling for adaptation-repositories
  // - check if repo is in right format (adaptation\d*)
  // - check if the configuration has that many adaptation repos
  // - export the entry in the adaptation list as %(adaptation)
  // - look up variables for that adaptation, and export matching
  //   adaptation variable
  QRegExp regex("adaptation\\\d*", Qt::CaseSensitive, QRegExp::RegExp2);
  if (regex.exactMatch(adaptationName)){
    regex.setPattern("\\\d*");
    regex.lastIndexIn(adaptationName);
    int n = regex.cap().toInt();

    if (!adaptationRepoList.isEmpty()){
      if (adaptationRepoList.size() <= n) {
        ssuLog->print(LOG_INFO, "Note: repo index out of bounds, substituting 0" + adaptationName);
        n = 0;
      }

      QString adaptationRepo = adaptationRepoList.at(n);
      storageHash->insert("adaptation", adaptationRepo);
      ssuLog->print(LOG_DEBUG, "Found first adaptation " + adaptationName);

      QHash<QString, QString> h = variableSection(adaptationRepo);

      QHash<QString, QString>::const_iterator i = h.constBegin();
      while (i != h.constEnd()){
        storageHash->insert(i.key(), i.value());
        i++;
      }
    } else
      ssuLog->print(LOG_INFO, "Note: adaptation repo for invalid repo requested " + adaptationName);

    return "adaptation";
  }
  return adaptationName;
}

QString SsuDeviceInfo::deviceFamily(){
  if (!cachedFamily.isEmpty())
    return cachedFamily;

  QString model = deviceVariant(true);

  cachedFamily = "UNKNOWN";

  if (boardMappings->contains(model + "/family"))
    cachedFamily = boardMappings->value(model + "/family").toString();

  return cachedFamily;
}

QString SsuDeviceInfo::deviceVariant(bool fallback){
  if (!cachedVariant.isEmpty())
    return cachedVariant;

  cachedVariant = "";

  if (boardMappings->contains("variants/" + deviceModel())) {
    cachedVariant = boardMappings->value("variants/" + deviceModel()).toString();
  }

  if (cachedVariant == "" && fallback)
    return deviceModel();

  return cachedVariant;
}

QString SsuDeviceInfo::deviceModel(){
  QDir dir;
  QFile procCpuinfo;
  QStringList keys;

  if (!cachedModel.isEmpty())
    return cachedModel;

  boardMappings->beginGroup("file.exists");
  keys = boardMappings->allKeys();

  // check if the device can be identified by testing for a file
  foreach (const QString &key, keys){
    QString value = boardMappings->value(key).toString();
    if (dir.exists(value)){
      cachedModel = key;
      break;
    }
  }
  boardMappings->endGroup();
  if (!cachedModel.isEmpty()) return cachedModel;

  // check if the QSystemInfo model is useful
  QSystemDeviceInfo devInfo;
  QString model = devInfo.model();
  boardMappings->beginGroup("systeminfo.equals");
  keys = boardMappings->allKeys();
  foreach (const QString &key, keys){
    QString value = boardMappings->value(key).toString();
    if (model == value){
      cachedModel = key;
      break;
    }
  }
  boardMappings->endGroup();
  if (!cachedModel.isEmpty()) return cachedModel;

  // check if the device can be identified by a string in /proc/cpuinfo
  procCpuinfo.setFileName("/proc/cpuinfo");
  procCpuinfo.open(QIODevice::ReadOnly | QIODevice::Text);
  if (procCpuinfo.isOpen()){
    QTextStream in(&procCpuinfo);
    QString cpuinfo = in.readAll();
    boardMappings->beginGroup("cpuinfo.contains");
    keys = boardMappings->allKeys();

    foreach (const QString &key, keys){
      QString value = boardMappings->value(key).toString();
      if (cpuinfo.contains(value)){
        cachedModel = key;
        break;
      }
    }
    boardMappings->endGroup();
  }
  if (!cachedModel.isEmpty()) return cachedModel;


  // check if there's a match on arch ofr generic fallback. This probably
  // only makes sense for x86
  boardMappings->beginGroup("arch.equals");
  keys = boardMappings->allKeys();

  SsuCoreConfig *settings = SsuCoreConfig::instance();
  foreach (const QString &key, keys){
    QString value = boardMappings->value(key).toString();
    if (settings->value("arch").toString() == value){
      cachedModel = key;
      break;
    }
  }
  boardMappings->endGroup();
  if (cachedModel.isEmpty()) cachedModel = "UNKNOWN";

  return cachedModel;
}

QString SsuDeviceInfo::deviceUid(){
  QString IMEI;
  QSystemDeviceInfo devInfo;

  IMEI = devInfo.imei();

  // this might not be completely unique (or might change on reflash), but works for now
  if (IMEI == ""){
      IMEI = devInfo.uniqueDeviceID();
  }

  return IMEI;
}

QStringList SsuDeviceInfo::disabledRepos(){
  QStringList result;

  QString model = deviceVariant(true);

  if (boardMappings->contains(model + "/disabled-repos"))
    result = boardMappings->value(model + "/disabled-repos").toStringList();

  return result;
}

// this half belongs into repo-manager, as it not only handles board-specific
// repositories. Right now this one looks like the better place due to the
// connection to board specific stuff, though
QStringList SsuDeviceInfo::repos(bool rnd, int filter){
  int adaptationCount = adaptationRepos().size();
  QStringList result;

  // for repo names we have adaptation0, adaptation1, ..., adaptationN
  for (int i=0; i<adaptationCount; i++)
    result.append(QString("adaptation%1").arg(i));

  if (filter == NoFilter || filter == BoardFilter || filter == BoardFilterUserBlacklist){
    // now read the release/rnd repos
    QSettings repoSettings(SSU_REPO_CONFIGURATION, QSettings::IniFormat);
    QString repoKey = (rnd ? "default-repos/rnd" : "default-repos/release");
    if (repoSettings.contains(repoKey))
      result.append(repoSettings.value(repoKey).toStringList());

    // TODO: add specific repos (developer, sdk, ..)

    // add device configured repos
    if (boardMappings->contains(deviceVariant(true) + "/repos"))
      result.append(boardMappings->value(deviceVariant(true) + "/repos").toStringList());

    // read the disabled repositories for this device
    // user can override repositories disabled here in the user configuration
    foreach (const QString &key, disabledRepos())
      result.removeAll(key);
  }

  // read user-defined repositories from ssu.ini
  // TODO: in strict mode, filter the repository list from there
  SsuCoreConfig *ssuSettings = SsuCoreConfig::instance();

  if (filter == NoFilter || filter == UserFilter){
    ssuSettings->beginGroup("repository-urls");
    result.append(ssuSettings->allKeys());
    ssuSettings->endGroup();

    // read user-enabled repositories from ssu.ini
    if (ssuSettings->contains("enabled-repos"))
      result.append(ssuSettings->value("enabled-repos").toStringList());
  }

  if (filter == NoFilter || filter == UserFilter || filter == BoardFilterUserBlacklist){
    // read the disabled repositories from user configuration
    if (ssuSettings->contains("disabled-repos")){
      foreach (const QString &key, ssuSettings->value("disabled-repos").toStringList())
        result.removeAll(key);
    }
  }

  result.removeDuplicates();
  return result;
}

QHash<QString, QString> SsuDeviceInfo::variableSection(QString section){
  QHash<QString, QString> result;

  if (!section.startsWith("var-"))
    section = "var-" + section;

  if (boardMappings->contains(section + "/variables")){
    return variableSection(boardMappings->value(section + "/variables").toString());
  }

  boardMappings->beginGroup(section);
  if (boardMappings->group() != section)
    return result;

  QStringList keys = boardMappings->allKeys();
  foreach (const QString &key, keys){
    result.insert(key, boardMappings->value(key).toString());
  }

  return result;
}

void SsuDeviceInfo::setDeviceModel(QString model){
  if (model == "")
    cachedModel = "";
  else
    cachedModel = model;

  cachedFamily = "";
  cachedVariant = "";
}

QVariant SsuDeviceInfo::value(const QString &key, const QVariant &value){
  if (boardMappings->contains(deviceVariant()+"/"+key)){
    return boardMappings->value(deviceVariant()+"/"+key);
  } else if (boardMappings->contains(deviceModel()+"/"+key)){
    return boardMappings->value(deviceModel()+"/"+key);
  }

  return value;
}
