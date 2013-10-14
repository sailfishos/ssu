/**
 * @file ssurepomanager.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include <QStringList>
#include <QRegExp>
#include <QDirIterator>

#include "sandbox_p.h"
#include "ssudeviceinfo.h"
#include "ssurepomanager.h"
#include "ssucoreconfig.h"
#include "ssusettings.h"
#include "ssulog.h"
#include "ssuvariables.h"
#include "ssu.h"

#include "../constants.h"

SsuRepoManager::SsuRepoManager(): QObject() {

}

void SsuRepoManager::add(QString repo, QString repoUrl){
  SsuCoreConfig *ssuSettings = SsuCoreConfig::instance();

  if (repoUrl == ""){
    // just enable a repository which has URL in repos.ini
    QStringList enabledRepos;
    if (ssuSettings->contains("enabled-repos"))
      enabledRepos = ssuSettings->value("enabled-repos").toStringList();

    enabledRepos.append(repo);
    enabledRepos.removeDuplicates();
    ssuSettings->setValue("enabled-repos", enabledRepos);
  } else
    ssuSettings->setValue("repository-urls/" + repo, repoUrl);

  ssuSettings->sync();
}

QString SsuRepoManager::caCertificatePath(QString domain){
  SsuCoreConfig *settings = SsuCoreConfig::instance();
  SsuSettings repoSettings(SSU_REPO_CONFIGURATION, QSettings::IniFormat);

  if (domain.isEmpty())
    domain = settings->domain();

  QString ca = SsuVariables::variable(&repoSettings, domain + "-domain",
                                      "_ca-certificate").toString();
  if (!ca.isEmpty())
    return ca;

  // compat setting, can go away after some time
  if (settings->contains("ca-certificate"))
    return settings->value("ca-certificate").toString();

  return "";
}

void SsuRepoManager::disable(QString repo){
  SsuCoreConfig *ssuSettings = SsuCoreConfig::instance();
  QStringList disabledRepos;

  if (ssuSettings->contains("disabled-repos"))
    disabledRepos = ssuSettings->value("disabled-repos").toStringList();

  disabledRepos.append(repo);
  disabledRepos.removeDuplicates();

  ssuSettings->setValue("disabled-repos", disabledRepos);
  ssuSettings->sync();
}

void SsuRepoManager::enable(QString repo){
  SsuCoreConfig *ssuSettings = SsuCoreConfig::instance();
  QStringList disabledRepos;

  if (ssuSettings->contains("disabled-repos"))
    disabledRepos = ssuSettings->value("disabled-repos").toStringList();

  disabledRepos.removeAll(repo);
  disabledRepos.removeDuplicates();

  ssuSettings->setValue("disabled-repos", disabledRepos);
  ssuSettings->sync();
}

void SsuRepoManager::remove(QString repo){
  SsuCoreConfig *ssuSettings = SsuCoreConfig::instance();
  if (ssuSettings->contains("repository-urls/" + repo))
    ssuSettings->remove("repository-urls/" + repo);

  if (ssuSettings->contains("enabled-repos")){
    QStringList enabledRepos = ssuSettings->value("enabled-repos").toStringList();
    if (enabledRepos.contains(repo)){
      enabledRepos.removeAll(repo);
      enabledRepos.removeDuplicates();
      ssuSettings->setValue("enabled-repos", enabledRepos);
    }
  }

  ssuSettings->sync();
}

QStringList SsuRepoManager::repos(bool rnd, int filter){
  SsuDeviceInfo deviceInfo;

  return repos(rnd, deviceInfo, filter);
}

QStringList SsuRepoManager::repos(bool rnd, SsuDeviceInfo &deviceInfo, int filter){
  QStringList result;
  result = deviceInfo.repos(rnd, filter);

  result.sort();
  result.removeDuplicates();

  return result;
}

void SsuRepoManager::update(){
  // - delete all non-ssu managed repositories (missing ssu_ prefix)
  // - create list of ssu-repositories for current adaptation
  // - go through ssu_* repositories, delete all which are not in the list; write others

  SsuDeviceInfo deviceInfo;
  QStringList ssuFilters;

  SsuCoreConfig *ssuSettings = SsuCoreConfig::instance();
  int deviceMode = ssuSettings->value("deviceMode").toInt();

  SsuLog *ssuLog = SsuLog::instance();

  // if device is misconfigured, always assume release mode
  bool rndMode = false;

  if ((deviceMode & Ssu::DisableRepoManager) == Ssu::DisableRepoManager){
    ssuLog->print(LOG_INFO, "Repo management requested, but not enabled (option 'deviceMode')");
    return;
  }

  if ((deviceMode & Ssu::RndMode) == Ssu::RndMode)
    rndMode = true;

  // get list of device-specific repositories...
  QStringList repos = deviceInfo.repos(rndMode);

  // strict mode enabled -> delete all repositories not prefixed by ssu
  // assume configuration error if there are no device repos, and don't delete
  // anything, even in strict mode
  if ((deviceMode & Ssu::LenientMode) != Ssu::LenientMode && !repos.isEmpty()){
    QDirIterator it(Sandbox::map(ZYPP_REPO_PATH), QDir::AllEntries|QDir::NoDot|QDir::NoDotDot);
    while (it.hasNext()){
      it.next();
      if (it.fileName().left(4) != "ssu_"){
        ssuLog->print(LOG_INFO, "Strict mode enabled, removing unmanaged repository " + it.fileName());
        QFile(it.filePath()).remove();
      }
    }
  }

  // ... delete all ssu-managed repositories not valid for this device ...
  ssuFilters.append("ssu_*");
  QDirIterator it(Sandbox::map(ZYPP_REPO_PATH), ssuFilters);
  while (it.hasNext()){
    QString f = it.next();

    QStringList parts = it.fileName().split("_");
    // repo file structure is ssu_<reponame>_<rnd|release>.repo -> splits to 3 parts
    if (parts.count() == 3){
      if (!repos.contains(parts.at(1)) ||
          parts.at(2) != (rndMode ? "rnd.repo" : "release.repo" ))
        QFile(it.filePath()).remove();
    } else
      QFile(it.filePath()).remove();
  }

  // ... and create all repositories required for this device
  foreach (const QString &repo, repos){
    // repo should be used where a unique identifier for silly human brains, or
    // zypper is required. repoName contains the shortened form for ssu use
    QString repoName = repo;
    QString debugSplit;
    if (repo.endsWith("-debuginfo")){
      debugSplit = "&debug";
      repoName = repo.left(repo.size() - 10);
    }

    QString repoFilePath = QString("%1/ssu_%2_%3.repo")
      .arg(Sandbox::map(ZYPP_REPO_PATH))
      .arg(repo)
      .arg(rndMode ? "rnd" : "release");

    if (url(repoName, rndMode) == ""){
      // TODO, repositories should only be disabled if they're not required
      //       for this machine. For required repositories error is better
      QTextStream qerr(stderr);
      qerr << "Repository " << repo << " does not contain valid URL, skipping and disabling." << endl;
      disable(repo);
      QFile(repoFilePath).remove();
      continue;
    }

    QFile repoFile(repoFilePath);

    if (repoFile.open(QIODevice::WriteOnly | QIODevice::Text)){
      QTextStream out(&repoFile);
      // TODO, add -rnd or -release if we want to support having rnd and
      //       release enabled at the same time
      out << "[" << repo << "]" << endl
          << "name=" << repo << endl
          << "failovermethod=priority" << endl
          << "type=rpm-md" << endl
          << "gpgcheck=0" << endl
          << "enabled=1" << endl;

      if (rndMode)
        out << "baseurl=plugin:ssu?rnd&repo=" << repoName << debugSplit << endl;
      else
        out << "baseurl=plugin:ssu?repo=" << repoName << debugSplit << endl;

      out.flush();
    }
  }
}

QStringList SsuRepoManager::repoVariables(QHash<QString, QString> *storageHash, bool rnd){
  SsuVariables var;
  SsuCoreConfig *settings = SsuCoreConfig::instance();
  QStringList configSections;
  SsuSettings repoSettings(SSU_REPO_CONFIGURATION, QSettings::IniFormat);

  // fill in all arbitrary variables from ssu.ini
  var.variableSection(settings, "repository-url-variables", storageHash);

  // add/overwrite some of the variables with sane ones
  if (rnd){
    storageHash->insert("flavour",
                          repoSettings.value(
                            settings->flavour()+"-flavour/flavour-pattern").toString());
    storageHash->insert("flavourPattern",
                          repoSettings.value(
                            settings->flavour()+"-flavour/flavour-pattern").toString());
    storageHash->insert("flavourName", settings->flavour());
    configSections << settings->flavour()+"-flavour" << "rnd" << "all";

    // Make it possible to give any values with the flavour as well.
    // These values can be overridden later with domain if needed.
    var.variableSection(&repoSettings, settings->flavour()+"-flavour", storageHash);
  } else {
    configSections << "release" << "all";
  }

  storageHash->insert("release", settings->release(rnd));

  if (!storageHash->contains("debugSplit"))
    storageHash->insert("debugSplit", "packages");

  if (!storageHash->contains("arch"))
    storageHash->insert("arch", settings->value("arch").toString());

  return configSections;
}

// RND repos have flavour (devel, testing, release), and release (latest, next)
// Release repos only have release (latest, next, version number)
QString SsuRepoManager::url(QString repoName, bool rndRepo,
                            QHash<QString, QString> repoParameters,
                            QHash<QString, QString> parametersOverride){
  QString r;
  QStringList configSections;
  SsuVariables var;
  SsuCoreConfig *settings = SsuCoreConfig::instance();
  SsuSettings repoSettings(SSU_REPO_CONFIGURATION, QSettings::IniFormat);
  SsuDeviceInfo deviceInfo;

  // set debugSplit for incorrectly configured debuginfo repositories (debugSplit
  // should already be passed by the url resolver); might be overriden later on,
  // if required
  if (repoName.endsWith("-debuginfo") && !repoParameters.contains("debugSplit"))
    repoParameters.insert("debugSplit", "debug");

  configSections = repoVariables(&repoParameters, rndRepo);


  // Override device model (and therefore all the family, ... stuff)
  if (parametersOverride.contains("model"))
    deviceInfo.setDeviceModel(parametersOverride.value("model"));

  repoParameters.insert("deviceFamily", deviceInfo.deviceFamily());
  repoParameters.insert("deviceModel", deviceInfo.deviceModel());

  repoName = deviceInfo.adaptationVariables(repoName, &repoParameters);


  QString domain;
  if (parametersOverride.contains("domain")){
    domain = parametersOverride.value("domain");
    domain.replace("-", ":");
  } else
    domain = settings->domain();

  // variableSection does autodetection for the domain default section
  var.variableSection(&repoSettings,
                      domain + "-domain", &repoParameters);

  // override arbitrary variables, mostly useful for generating mic URLs
  QHash<QString, QString>::const_iterator i = parametersOverride.constBegin();
  while (i != parametersOverride.constEnd()){
    repoParameters.insert(i.key(), i.value());
    i++;
  }

  if (settings->contains("repository-urls/" + repoName))
    r = settings->value("repository-urls/" + repoName).toString();
  else {
    foreach (const QString &section, configSections){
      repoSettings.beginGroup(section);
      if (repoSettings.contains(repoName)){
        r = repoSettings.value(repoName).toString();
        repoSettings.endGroup();
        break;
      }
      repoSettings.endGroup();
    }
  }

  return var.resolveString(r, &repoParameters);
}
