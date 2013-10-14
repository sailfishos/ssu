/**
 * @file ssudeviceinfo.cpp
 * @copyright 2013 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */


#include <QTextStream>
#include <QDir>

#include <sys/utsname.h>

extern "C" {
#include <boardname.h>
}

#include "sandbox_p.h"
#include "ssudeviceinfo.h"
#include "ssucoreconfig.h"
#include "ssulog.h"
#include "ssuvariables.h"

#include "../constants.h"

#include <QDeviceInfo>

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

      QString model = deviceVariant(true);
      QHash<QString, QString> h;

      // add global variables for this model
      if (boardMappings->contains(model + "/variables")){
        QStringList sections = boardMappings->value(model + "/variables").toStringList();
        foreach(const QString &section, sections)
          variableSection(section, &h);
      }

      // override with variables specific to this repository
      variableSection(adaptationRepo, &h);

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

void SsuDeviceInfo::clearCache(){
  cachedFamily = "";
  cachedModel = "";
  cachedVariant = "";
}

bool SsuDeviceInfo::contains(const QString &model){
  QString oldModel = deviceModel();
  bool found = false;

  if (!model.isEmpty()){
    clearCache();
    setDeviceModel(model);
  }

  if (!deviceVariant(false).isEmpty())
    found = true;
  if (boardMappings->childGroups().contains(deviceModel()))
    found = true;

  if (!model.isEmpty()){
    clearCache();
    setDeviceModel(oldModel);
  }
  return found;
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
  QStringList sections;

  if (!cachedModel.isEmpty())
    return cachedModel;

  boardMappings->beginGroup("file.exists");
  keys = boardMappings->allKeys();

  // check if the device can be identified by testing for a file
  foreach (const QString &key, keys){
    QString value = boardMappings->value(key).toString();
    if (dir.exists(Sandbox::map(value))){
      cachedModel = key;
      break;
    }
  }
  boardMappings->endGroup();
  if (!cachedModel.isEmpty()) return cachedModel;

  // check if boardname matches/contains
  QString boardName(getboardname());
  boardName = boardName.trimmed();
  sections.clear();
  sections << "boardname.equals" << "boardname.contains";
  foreach (const QString &section, sections){
    boardMappings->beginGroup(section);
    keys = boardMappings->allKeys();
    foreach (const QString &key, keys){
      QString value = boardMappings->value(key).toString();
      if (section.endsWith(".contains")){
        if (boardName.contains(value)){
          cachedModel = key;
          break;
        }
      } else if (section.endsWith(".equals")){
        if (boardName == value){
          cachedModel = key;
          break;
        }
      }
    }
    boardMappings->endGroup();
    if (!cachedModel.isEmpty()) break;
  }
  if (!cachedModel.isEmpty()) return cachedModel;

  // check if the QSystemInfo model is useful
  //QSystemDeviceInfo devInfo;
  // TODO Current Mer SystemDeviceInfo only returns cpuinfo stuff,
  //      which is what we can do with cpuinfo matching in a more
  //      flexible way, so there's not really any need to pull in the
  //      whole X11 stack just for this. Can be enabled once systeminfo
  //      is less insane
  /*
  QSystemDeviceInfoLinuxCommonPrivate devInfo;
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
  */

  // check if the device can be identified by a string in /proc/cpuinfo
  procCpuinfo.setFileName(Sandbox::map("/proc/cpuinfo"));
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

  // check if the device can be identified by the kernel version string
  struct utsname buf;
  if (!uname(&buf)){
    QString utsRelease(buf.release);
    boardMappings->beginGroup("uname-release.contains");
    keys = boardMappings->allKeys();

    foreach (const QString &key, keys){
      QString value = boardMappings->value(key).toString();
      if (utsRelease.contains(value)){
        cachedModel = key;
        break;
      }
    }
    boardMappings->endGroup();
  }
  if (!cachedModel.isEmpty()) return cachedModel;

  // check if there's a match on arch of generic fallback. This probably
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
  QDeviceInfo devInfo;

  /// @todo properly check number of imeis, ...
  IMEI = devInfo.imei(0);

  // this might not be completely unique (or might change on reflash), but works for now
  if (IMEI == ""){
      IMEI = devInfo.uniqueDeviceID();
      IMEI.replace("-", "");
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

  if (filter == Ssu::NoFilter ||
      filter == Ssu::BoardFilter ||
      filter == Ssu::BoardFilterUserBlacklist){
    // for repo names we have adaptation0, adaptation1, ..., adaptationN
    for (int i=0; i<adaptationCount; i++)
      result.append(QString("adaptation%1").arg(i));

    // now read the release/rnd repos
    SsuSettings repoSettings(SSU_REPO_CONFIGURATION, QSettings::IniFormat);
    QString repoKey = (rnd ? "default-repos/rnd" : "default-repos/release");
    if (repoSettings.contains(repoKey))
      result.append(repoSettings.value(repoKey).toStringList());

    // TODO: add specific repos (developer, sdk, ..)

    // add device configured repos
    if (boardMappings->contains(deviceVariant(true) + "/repos"))
      result.append(boardMappings->value(deviceVariant(true) + "/repos").toStringList());

    // add device configured repos only valid for rnd and/or release
    repoKey = (rnd ? "/repos-rnd" : "/repos-release");
    if (boardMappings->contains(deviceVariant(true) + repoKey))
      result.append(boardMappings->value(deviceVariant(true) + repoKey).toStringList());

    // read the disabled repositories for this device
    // user can override repositories disabled here in the user configuration
    foreach (const QString &key, disabledRepos())
      result.removeAll(key);
  }

  // read user-defined repositories from ssu.ini
  // TODO: in strict mode, filter the repository list from there
  SsuCoreConfig *ssuSettings = SsuCoreConfig::instance();

  if (filter == Ssu::NoFilter ||
      filter == Ssu::UserFilter){
    ssuSettings->beginGroup("repository-urls");
    result.append(ssuSettings->allKeys());
    ssuSettings->endGroup();

    // read user-enabled repositories from ssu.ini
    if (ssuSettings->contains("enabled-repos"))
      result.append(ssuSettings->value("enabled-repos").toStringList());
  }

  if (filter == Ssu::NoFilter ||
      filter == Ssu::UserFilter ||
      filter == Ssu::BoardFilterUserBlacklist){
    // read the disabled repositories from user configuration
    if (ssuSettings->contains("disabled-repos")){
      foreach (const QString &key, ssuSettings->value("disabled-repos").toStringList())
        result.removeAll(key);
    }
  }

  result.removeDuplicates();
  return result;
}

QVariant SsuDeviceInfo::variable(QString section, const QString &key){
  /// @todo compat-setting as ssudeviceinfo guaranteed to prepend sections with var-;
  ///       SsuVariables does not have this guarantee. Remove from here as well.
  if (!section.startsWith("var-"))
    section = "var-" + section;

  return SsuVariables::variable(boardMappings, section, key);
}

void SsuDeviceInfo::variableSection(QString section, QHash<QString, QString> *storageHash){
  if (!section.startsWith("var-"))
    section = "var-" + section;

  SsuVariables::variableSection(boardMappings, section, storageHash);
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
