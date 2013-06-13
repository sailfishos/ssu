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

QStringList SsuKickstarter::commandSection(const QString &section, const QString &description){
  QStringList result;
  SsuDeviceInfo deviceInfo(deviceModel);
  QString commandFile;
  QFile part;

  QDir dir(Sandbox::map(QString("/%1/kickstart/%2/")
                        .arg(SSU_DATA_DIR)
                        .arg(section)));

  if (dir.exists(replaceSpaces(deviceModel.toLower())))
    commandFile = replaceSpaces(deviceModel.toLower());
  else if (dir.exists(replaceSpaces(deviceInfo.deviceVariant(true).toLower())))
    commandFile = replaceSpaces(deviceInfo.deviceVariant(true).toLower());
  else if (dir.exists("default"))
    commandFile = "default";
  else {
    if (description.isEmpty())
      result.append("## No suitable configuration found in " + dir.path());
    else
      result.append("## No configuration for " + description + " found.");
    return result;
  }

  QFile file(dir.path() + "/" + commandFile);

  if (description.isEmpty())
    result.append("### Commands from " + dir.path() + "/" + commandFile);
  else
    result.append("### " + description + " from " + commandFile);

  if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
    QTextStream in(&file);
    while (!in.atEnd())
      result.append(in.readLine());
  }

  return result;
}

QString SsuKickstarter::replaceSpaces(const QString &value){
  QString retval = value;
  return retval.replace(" ", "_");
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
      result.append(QString("repo --name=%1-%2-%3 --baseurl=%4")
                    .arg(repo)
                    .arg(replaceSpaces(deviceModel))
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
    .arg(repoOverride.value("brand"))
    .arg(deviceModel);
  result.append(configuration);

  result.sort();
  result.removeDuplicates();
  result.prepend("%packages");
  result.append("%end");
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
  QStringList commandSections;

  // initialize with default 'part' for compatibility, as partitions
  // used to work without configuration. It'll get replaced with
  // configuration values, if found
  commandSections.append("part");

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
  if (deviceInfo.variable("kickstart-defaults", "commandSections")
      .canConvert(QMetaType::QStringList)){
    commandSections =
      deviceInfo.variable("kickstart-defaults", "commandSections").toStringList();
  }

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

  // do sanity checking on the model
  if (deviceInfo.contains() == false) {
    qerr << "Device model '" << deviceInfo.deviceModel() << "' does not exist" << endl;

    if (repoOverride.value("force") != "true")
      return false;
  }

  QRegExp regex(" {2,}", Qt::CaseSensitive, QRegExp::RegExp2);
  if (regex.indexIn(deviceInfo.deviceModel(), 0) != -1){
    qerr << "Device model '" << deviceInfo.deviceModel()
         << "' contains multiple consecutive spaces." << endl;
    if (deviceInfo.contains())
      qerr << "Since the model exists it looks like your configuration is broken." << endl;
    return false;
  }

  if (!repoOverride.contains("brand")){
    qerr << "No brand set. Check your configuration." << endl;
    return false;
  }

  bool opened = false;
  QString outputDir = repoOverride.value("outputdir");
  if (!outputDir.isEmpty()) outputDir.append("/");

  if (kickstart.isEmpty()){
    if (repoOverride.contains("filename")){
      QString fileName = QString("%1%2")
        .arg(outputDir)
        .arg(replaceSpaces(var.resolveString(repoOverride.value("filename"),
                                             &repoOverride)));

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
  foreach (const QString &section, commandSections){
    kout << commandSection(section).join("\n") << endl << endl;
  }
  kout << repos().join("\n") << endl << endl;
  kout << packages().join("\n") << endl << endl;
  kout << scriptletSection("pre", true).join("\n") << endl << endl;
  kout << scriptletSection("post", true).join("\n") << endl << endl;
  kout << scriptletSection("post", false).join("\n") << endl << endl;
  // add flags as bitmask?
  // POST, die-on-error

  return true;
}
