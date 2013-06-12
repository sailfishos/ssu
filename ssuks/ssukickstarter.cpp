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
#include "libssu/sandbox_p.h"
#include "libssu/ssurepomanager.h"
#include "libssu/ssuvariables.h"

#include "../constants.h"

/* TODO:
 * - commands from the command section should be verified
 * - allow overriding brand key
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

  QHash<QString, QString> h;

  deviceInfo.variableSection("kickstart-commands", &h);

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
    // Adaptation repos need to have separate naming so that when images are done
    // the repository caches will not be mixed with each other.
    if (repo.startsWith("adaptation")) {
      // make sure device model doesn't introduce spaces to the reponame
      QString deviceModelTmp = deviceModel;
      deviceModelTmp.replace(" ","-");
      result.append(QString("repo --name=%1-%2-%3 --baseurl=%4")
                    .arg(repo)
                    .arg(deviceModelTmp)
                    .arg((rndMode ? repoOverride.value("rndRelease")
                          : repoOverride.value("release")))
                    .arg(repoUrl)
        );
    }
    else
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

  QDir dir(Sandbox::map(QString("/%1/kickstart/part/")
           .arg(SSU_DATA_DIR)));

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
    path = Sandbox::map(QString("/%1/kickstart/%2/")
      .arg(SSU_DATA_DIR)
      .arg(name));
  else
    path = Sandbox::map(QString("/%1/kickstart/%2_nochroot/")
      .arg(SSU_DATA_DIR)
      .arg(name));

  dir.setPath(path);
  QStringList scriptlets = dir.entryList(QDir::AllEntries|QDir::NoDot|QDir::NoDotDot,
                                         QDir::Name);

  foreach (const QString &scriptlet, scriptlets){
    QFile file(dir.filePath(scriptlet));
    result.append("### begin " + scriptlet);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
      QTextStream in(&file);
      while (!in.atEnd())
        result.append(in.readLine());
    }
    result.append("### end " + scriptlet);
  }

  if (!result.isEmpty()){
    result.prepend(QString("export SSU_RELEASE_TYPE=%1")
                   .arg(rndMode ? "rnd" : "release"));

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
}

bool SsuKickstarter::write(QString kickstart){
  QFile ks;
  QTextStream kout;
  QTextStream qerr(stderr);
  SsuDeviceInfo deviceInfo(deviceModel);
  SsuRepoManager repoManager;
  SsuVariables var;

  // rnd mode should not come from the defaults
  if (repoOverride.contains("rnd")){
    if (repoOverride.value("rnd") == "true")
      rndMode = true;
    else if (repoOverride.value("rnd") == "false")
      rndMode = false;
  }

  QHash<QString, QString> defaults;
  // get generic repo variables; domain and adaptation specific bits are not interesting
  // in the kickstart
  repoManager.repoVariables(&defaults, rndMode);

  // overwrite with kickstart defaults
  deviceInfo.variableSection("kickstart-defaults", &defaults);

  QHash<QString, QString>::const_iterator it = defaults.constBegin();
  while (it != defaults.constEnd()){
    if (!repoOverride.contains(it.key()))
      repoOverride.insert(it.key(), it.value());
    it++;
  }

  // in rnd mode both rndRelease an release should be the same,
  // as the variable name used is %(release)
  if (rndMode && repoOverride.contains("rndRelease"))
    repoOverride.insert("release", repoOverride.value("rndRelease"));

  // release mode variables should not contain flavourName
  if (!rndMode && repoOverride.contains("flavourName"))
    repoOverride.remove("flavourName");

  //TODO: check for mandatory keys, brand, ..
  if (!repoOverride.contains("deviceModel"))
    repoOverride.insert("deviceModel", deviceInfo.deviceModel());

  bool opened = false;
  QString outputDir = repoOverride.value("outputdir");
  if (!outputDir.isEmpty()) outputDir.append("/");

  if (kickstart.isEmpty()){
    if (repoOverride.contains("filename")){
      QString fileName = QString("%1%2")
                                 .arg(outputDir)
                                 .arg(var.resolveString(repoOverride.value("filename"), &repoOverride).replace(" ","-"));

      ks.setFileName(fileName);
      opened = ks.open(QIODevice::WriteOnly);
    } else {
      qerr << "No filename specified, and no default filename configured" << endl;
      return false;
    }
  } else if (kickstart == "-")
    opened = ks.open(stdout, QIODevice::WriteOnly);
  else {
    ks.setFileName(outputDir + kickstart);
    opened = ks.open(QIODevice::WriteOnly);
  }

  if (!opened) {
    qerr << "Unable to write output file " << ks.fileName() << ": " << ks.errorString() << endl;
    return false;
  }

  QString displayName = QString("# DisplayName: %1 %2/%3 (%4) %5")
                                .arg(repoOverride.value("brand"))
                                .arg(deviceInfo.deviceModel())
                                .arg(repoOverride.value("arch"))
                                .arg((rndMode ? "rnd"
                                              : "release"))
                                .arg(repoOverride.value("version"));

  kout.setDevice(&ks);
  kout << displayName << endl << endl;

  kout << commands().join("\n") << endl << endl;
  kout << partitions().join("\n") << endl << endl;
  kout << repos().join("\n") << endl << endl;
  kout << packages().join("\n") << endl << endl;
  kout << scriptletSection("pre", true).join("\n") << endl << endl;
  kout << scriptletSection("post", true).join("\n") << endl << endl;
  kout << scriptletSection("post", false).join("\n") << endl << endl;
  // add flags as bitmask?
  // POST, die-on-error

  return true;
}
