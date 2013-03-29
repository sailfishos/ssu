/**
 * @file ssurepomanager.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include <QStringList>
#include <QRegExp>
#include <QDirIterator>

#include "ssudeviceinfo.h"
#include "ssurepomanager.h"
#include "ssulog.h"
#include "ssu.h"

#include "../constants.h"

SsuRepoManager::SsuRepoManager(): QObject() {

}

void SsuRepoManager::update(){
  // - delete all non-ssu managed repositories (missing ssu_ prefix)
  // - create list of ssu-repositories for current adaptation
  // - go through ssu_* repositories, delete all which are not in the list; write others

  SsuDeviceInfo deviceInfo;
  QStringList ssuFilters;

  QSettings ssuSettings(SSU_CONFIGURATION, QSettings::IniFormat);
  int deviceMode = ssuSettings.value("deviceMode").toInt();

  SsuLog *ssuLog = SsuLog::instance();

  // if device is misconfigured, always assume release mode
  bool rndMode = false;

  if ((deviceMode & Ssu::DisableRepoManager) == Ssu::DisableRepoManager){
    ssuLog->print(LOG_INFO, "Repo management requested, but not enabled (option 'deviceMode')");
    return;
  }

  if ((deviceMode & Ssu::RndMode) == Ssu::RndMode)
    rndMode = true;

  // strict mode enabled -> delete all repositories not prefixed by ssu_
  if ((deviceMode & Ssu::StrictMode) == Ssu::StrictMode){
    QDirIterator it(ZYPP_REPO_PATH, QDir::AllEntries|QDir::NoDot|QDir::NoDotDot);
    while (it.hasNext()){
      it.next();
      if (it.fileName().left(4) != "ssu_"){
        ssuLog->print(LOG_INFO, "Strict mode enabled, removing unmanaged repository " + it.fileName());
        QFile(it.filePath()).remove();
      }
    }
  }

  // get list of device-specific repositories...
  QStringList repos = deviceInfo.repos(rndMode);

  // ... delete all ssu-managed repositories not valid for this device ...
  ssuFilters.append("ssu_*");
  QDirIterator it(ZYPP_REPO_PATH, ssuFilters);
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
    QFile repoFile(QString("%1/ssu_%2_%3.repo")
                   .arg(ZYPP_REPO_PATH)
                   .arg(repo)
                   .arg(rndMode ? "rnd" : "release"));

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
        out << "baseurl=plugin:ssu?rnd&repo=" << repo << endl;
      else
        out << "baseurl=plugin:ssu?repo=" << repo << endl;

      out.flush();
    }
  }
}
