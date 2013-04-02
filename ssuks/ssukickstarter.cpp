/**
 * @file ssukickstarter.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include <QStringList>
#include <QRegExp>
#include <QDirIterator>

#include "ssukickstarter.h"

#include "../constants.h"

/* TODO:
 * - rnd/release handling.
 *   repo --name=adaptation0-@RNDRELEASE@
 *        --baseurl=https://releases.jollamobile.com/nemo/@RELEASE@-devel/adaptation-n9xx-common/@ARCH@/
 *   @RELEASE@ should be @RNDRELEASE@ in this case as well
 * - commands from the command section should be verified
 * - go through the variable lookup process for non-url variables as well
 * - allow overriding brand key
 * - use (and expand) filename key if no filename is given on command line
 */


SsuKickstarter::SsuKickstarter() {
  SsuDeviceInfo deviceInfo;
  deviceModel = deviceInfo.deviceModel();

  if ((ssu.deviceMode() & Ssu::RndMode) == Ssu::RndMode)
    rndMode = true;
  else
    rndMode = false;
}

QStringList SsuKickstarter::commands(){
  SsuDeviceInfo deviceInfo(deviceModel);
  QStringList result;

  QHash<QString, QString> h = deviceInfo.variableSection("kickstart-commands");

  // read commands from variable, ...

  QHash<QString, QString>::const_iterator it = h.constBegin();
  while (it != h.constEnd()){
    result.append(it.key() + " " + it.value());
    it++;
  }

  return result;
}

QStringList SsuKickstarter::repos(){
  QStringList result;
  SsuDeviceInfo deviceInfo(deviceModel);

  QStringList repos = deviceInfo.repos(rndMode, SsuDeviceInfo::BoardFilter);

  foreach (const QString &repo, repos){
    QString repoUrl = ssu.repoUrl(repo, rndMode, QHash<QString, QString>(), repoOverride);
    result.append(QString("repo --name=%1-%2 --baseurl=%3")
                  .arg(repo)
                  .arg((rndMode ? repoOverride.value("rndRelease")
                                : repoOverride.value("release")))
                  .arg(repoUrl)
      );
  }

  return result;
}

QStringList SsuKickstarter::packages(){
  QStringList result;

  // insert @vendor configuration device
  QString configuration = QString("@%1 Configuration %2")
    .arg("Jolla")
    .arg(deviceModel);
  result.append(configuration);

  // check if there's a kernel, and if so, append it as well

  result.sort();
  result.removeDuplicates();
  result.prepend("%packages");
  result.append("%end");
  return result;
}

QStringList SsuKickstarter::partitions(){
  QStringList result;
  SsuDeviceInfo deviceInfo(deviceModel);
  QString partitionFile;
  QFile part;

  QDir dir(QString("%1/kickstart/part/").arg(SSU_DATA_DIR));

  if (dir.exists(deviceModel.toLower()))
    partitionFile = deviceModel.toLower();
  else if (dir.exists(deviceInfo.deviceVariant(true).toLower()))
    partitionFile = deviceInfo.deviceVariant(true).toLower();
  else if (dir.exists("default"))
    partitionFile = "default";
  else {
    result.append("## No partition configuration found.");
    return result;
  }

  QFile file(dir.path() + "/" + partitionFile);
  result.append("### partition setup from " + partitionFile);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
    QTextStream in(&file);
    while (!in.atEnd())
      result.append(in.readLine());
  }

  return result;
}

// we intentionally don't support device-specific post scriptlets
QStringList SsuKickstarter::scriptletSection(QString name, bool chroot){
  QStringList result;
  QString path;
  QDir dir;

  if (chroot)
    path = QString("%1/kickstart/%2/")
      .arg(SSU_DATA_DIR)
      .arg(name);
  else
    path = QString("%1/kickstart/%2_nochroot/")
      .arg(SSU_DATA_DIR)
      .arg(name);

  dir.setPath(path);
  QStringList scriptlets = dir.entryList(QDir::AllEntries|QDir::NoDot|QDir::NoDotDot,
                                         QDir::Name);

  foreach (const QString &scriptlet, scriptlets){
    QFile file(path + scriptlet);
    result.append("### begin " + scriptlet);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
      QTextStream in(&file);
      while (!in.atEnd())
        result.append(in.readLine());
    }
    result.append("### end " + scriptlet);
  }

  if (!result.isEmpty()){
    if (chroot)
      result.prepend("%" + name);
    else
      result.prepend("%" + name + " --nochroot");
    result.append("%end");
  }

  return result;
}

void SsuKickstarter::setRepoParameters(QHash<QString, QString> parameters){
  repoOverride = parameters;

  if (repoOverride.contains("model"))
    deviceModel = repoOverride.value("model");
/*
  else
    repoOverride.insert("model", deviceInfo.deviceModel());
*/
}

void SsuKickstarter::write(QString kickstart){
  QFile ks;
  QTextStream kout;
  SsuDeviceInfo deviceInfo;

  if (kickstart.isEmpty())
    ks.open(stdout, QIODevice::WriteOnly);
  else {
    ks.setFileName(kickstart);
    ks.open(QIODevice::WriteOnly);
  }

  kout.setDevice(&ks);

  QHash<QString, QString> defaults = deviceInfo.variableSection("kickstart-defaults");
  QHash<QString, QString>::const_iterator it = defaults.constBegin();
  while (it != defaults.constEnd()){
    if (!repoOverride.contains(it.key()))
      repoOverride.insert(it.key(), it.value());
    it++;
  }

  if (repoOverride.contains("rnd")){
    if (repoOverride.value("rnd") == "true")
      rndMode = true;
    else if (repoOverride.value("rnd") == "false")
      rndMode = false;
  }

  // in rnd mode both rndRelease an release should be the same,
  // as the variable name used is %(release)
  if (rndMode && repoOverride.contains("rndRelease"))
    repoOverride.insert("release", repoOverride.value("rndRelease"));

  kout << commands().join("\n") << endl << endl;
  kout << partitions().join("\n") << endl << endl;
  kout << repos().join("\n") << endl << endl;
  kout << packages().join("\n") << endl << endl;
  kout << scriptletSection("pre", true).join("\n") << endl << endl;
  kout << scriptletSection("post", true).join("\n") << endl << endl;
  kout << scriptletSection("post", false).join("\n") << endl << endl;
  // add flags as bitmask?
  // POST, die-on-error
}
