/**
 * @file rndssucli.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#include <QCoreApplication>

#include <termios.h>

#include "libssu/ssudeviceinfo.h"
#include "libssu/ssurepomanager.h"
#include "libssu/ssucoreconfig.h"

#include <QDebug>

#include "rndssucli.h"

RndSsuCli::RndSsuCli(): QObject(){
  connect(this,SIGNAL(done()),
          QCoreApplication::instance(),SLOT(quit()), Qt::DirectConnection);
  connect(&ssu, SIGNAL(done()),
          this, SLOT(handleResponse()));
  state = Idle;
}

void RndSsuCli::handleResponse(){
  QTextStream qout(stdout);

  if (ssu.error()){
    qout << "Last operation failed: \n" << ssu.lastError() << endl;
    QCoreApplication::exit(1);
  } else {
    qout << "Operation successful" << endl;
    QCoreApplication::exit(0);
  }
}


void RndSsuCli::optFlavour(QStringList opt){
  QTextStream qout(stdout);

  if (opt.count() == 3){
    qout << "Changing flavour from " << ssu.flavour()
         << " to " << opt.at(2) << endl;
    ssu.setFlavour(opt.at(2));
    state = Idle;
  } else if (opt.count() == 2) {
    qout << "Device flavour is currently: " << ssu.flavour() << endl;
    state = Idle;
  }
}

void RndSsuCli::optMode(QStringList opt){
  QTextStream qout(stdout);

  // TODO: allow setting meaningful names instead of numbers

  if (opt.count() == 2){
    QStringList modeList;
    int deviceMode = ssu.deviceMode();

    if ((deviceMode & Ssu::DisableRepoManager) == Ssu::DisableRepoManager)
      modeList.append("DisableRepoManager");
    if ((deviceMode & Ssu::RndMode) == Ssu::RndMode)
      modeList.append("RndMode");
    if ((deviceMode & Ssu::ReleaseMode) == Ssu::ReleaseMode)
      modeList.append("ReleaseMode");
    if ((deviceMode & Ssu::LenientMode) == Ssu::LenientMode)
      modeList.append("LenientMode");

    qout << "Device mode is: " << ssu.deviceMode()
         << " (" << modeList.join(" | ") << ")" << endl;

    if ((deviceMode & Ssu::RndMode) == Ssu::RndMode &&
        (deviceMode & Ssu::ReleaseMode) == Ssu::ReleaseMode)
      qout << "Both Release and RnD mode set, device is in RnD mode" << endl;

    state = Idle;
  } else if (opt.count() == 3){
    qout << "Setting device mode from " << ssu.deviceMode()
         << " to " << opt.at(2) << endl;
    ssu.setDeviceMode(opt.at(2).toInt());
    state = Idle;
  }
}

void RndSsuCli::optModifyRepo(int action, QStringList opt){
  SsuRepoManager repoManager;
  QTextStream qout(stdout);

  if (opt.count() == 3){
    switch(action){
      case Add:
        repoManager.add(opt.at(2));
        repoManager.update();
        break;
      case Remove:
        repoManager.remove(opt.at(2));
        repoManager.update();
        break;
      case Disable:
        repoManager.disable(opt.at(2));
        repoManager.update();
        break;
      case Enable:
        repoManager.enable(opt.at(2));
        repoManager.update();
        break;
    }
  } else if (opt.count() == 4 && action == Add){
    repoManager.add(opt.at(2), opt.at(3));
    repoManager.update();
  }
}

void RndSsuCli::optRegister(){
  /*
   * register a new device
   */

  QString username, password;
  QTextStream qin(stdin);
  QTextStream qout(stdout);

  struct termios termNew, termOld;

  qout << "Username: " << flush;
  username = qin.readLine();

  tcgetattr(STDIN_FILENO, &termNew);
  termOld = termNew;
  termNew.c_lflag &= ~ECHO;
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &termNew) == -1)
    qout << "WARNING: Unable to disable echo on your terminal, password will echo!" << endl;

  qout << "Password: " << flush;
  password = qin.readLine();

  tcsetattr(STDIN_FILENO, TCSANOW, &termOld);

  ssu.sendRegistration(username, password);
  state = Busy;
}

void RndSsuCli::optRelease(QStringList opt){
  QTextStream qout(stdout);

  if (opt.count() == 3){
    if (opt.at(2) == "-r"){
      qout << "Device release (RnD) is currently: " << ssu.release(true) << endl;
      state = Idle;
    } else {
      qout << "Changing release from " << ssu.release()
           << " to " << opt.at(2) << endl;
      qout << "Your device is now in release mode!" << endl;
      ssu.setRelease(opt.at(2));
      state = Idle;
    }
  } else if (opt.count() == 2) {
    qout << "Device release is currently: " << ssu.release() << endl;
    state = Idle;
  } else if (opt.count() == 4 && opt.at(2) == "-r"){
    qout << "Changing release (RnD) from " << ssu.release(true)
         << " to " << opt.at(3) << endl;
    qout << "Your device is now in RnD mode!" << endl;
    ssu.setRelease(opt.at(3), true);
    state = Idle;
  }
}

void RndSsuCli::optRepos(QStringList opt){
  QTextStream qout(stdout);
  SsuRepoManager repoManager;
  SsuDeviceInfo deviceInfo;
  QHash<QString, QString> repoParameters, repoOverride;
  QString device="";
  bool rndRepo = false;
  int micMode=0, flagStart = 0;

  if ((ssu.deviceMode() & Ssu::RndMode) == Ssu::RndMode)
    rndRepo = true;

  if (opt.count() >= 3 && opt.at(2) == "-m"){
    micMode = 1;
    // TODO: read the default mic override variables from some config
    /*
    repoOverride.insert("release", "@RELEASE@");
    repoOverride.insert("rndRelease", "@RNDRELEASE@");
    repoOverride.insert("flavour", "@FLAVOUR@");
    repoOverride.insert("arch", "@ARCH@");
    */
  }

  if (opt.count() >= 3 + micMode){
    // first argument is flag
    if (opt.at(2 + micMode).contains("=")){
      flagStart = 2 + micMode;
    } else if (!opt.at(2 + micMode).contains("=") &&
               opt.count() == 3 + micMode) {
      // first (and only) argument is device)
      device = opt.at(2 + micMode);
    } else if(!opt.at(2 + micMode).contains("=") &&
              opt.count() > 3 + micMode &&
              opt.at(3 + micMode).contains("=")){
      // first argument is device, second flag
      device = opt.at(2 + micMode);
      flagStart = 3 + micMode;
    } else {
      state = UserError;
      return;
    }
  }

  if (flagStart != 0){
    for (int i=flagStart; i<opt.count(); i++){
      if (opt.at(i).count("=") != 1){
        qout << "Invalid flag: " << opt.at(i) << endl;
        state = Idle;
        return;
      }
      QStringList split = opt.at(i).split("=");
      repoOverride.insert(split.at(0), split.at(1));
    }
  }

  if (repoOverride.contains("rnd")){
    if (repoOverride.value("rnd") == "true")
      rndRepo = true;
    else if (repoOverride.value("rnd") == "false")
      rndRepo = false;
  }

  if (device != ""){
    deviceInfo.setDeviceModel(device);
    repoOverride.insert("model", device);
  }

  // TODO: rnd mode override needs to be implemented
  QStringList repos;

  // micMode? handle it and return, as it's a lot simpler than full mode
  if (micMode){
    repos = deviceInfo.repos(rndRepo, SsuDeviceInfo::BoardFilter);
    foreach (const QString &repo, repos){
      QString repoUrl = ssu.repoUrl(repo, rndRepo, repoParameters, repoOverride);
      qout << "repo --name=" << repo << "-"
           << (rndRepo ? repoOverride.value("rndRelease")
                       : repoOverride.value("release"))
           << " --baseurl=" << repoUrl << endl;
    }
    state = Idle;
    return;
  }

  if (device.isEmpty())
    repos = deviceInfo.repos(rndRepo, SsuDeviceInfo::BoardFilterUserBlacklist);
  else {
    qout << "Printing repository configuration for '" << device << "'" << endl << endl;
    repos = deviceInfo.repos(rndRepo, SsuDeviceInfo::BoardFilter);
  }

  SsuCoreConfig *ssuSettings = SsuCoreConfig::instance();

  qout << "Enabled repositories (global): " << endl;
  for (int i=0; i<=3; i++){
    // for each repository, print repo and resolve url
    int longestField = 0;
    foreach (const QString &repo, repos)
      if (repo.length() > longestField)
        longestField = repo.length();

    qout.setFieldAlignment(QTextStream::AlignLeft);

    foreach (const QString &repo, repos){
      QString repoUrl = ssu.repoUrl(repo, rndRepo, repoParameters, repoOverride);
      qout << " - " << qSetFieldWidth(longestField) << repo << qSetFieldWidth(0) << " ... " << repoUrl << endl;
    }

    if (i==0){
      if (device != ""){
        repos.clear();
        continue;
      }
      repos = deviceInfo.repos(rndRepo, SsuDeviceInfo::UserFilter);
      qout << endl << "Enabled repositories (user): " << endl;
    } else if (i==1){
      repos = deviceInfo.disabledRepos();
      if (device.isEmpty())
        qout << endl << "Disabled repositories (global, might be overridden by user config): " << endl;
      else
        qout << endl << "Disabled repositories (global): " << endl;
    } else if (i==2) {
      repos.clear();
      if (device != "")
        continue;
      if (ssuSettings->contains("disabled-repos"))
        repos.append(ssuSettings->value("disabled-repos").toStringList());
      qout << endl << "Disabled repositories (user): " << endl;
    }
  }

  state = Idle;
}

void RndSsuCli::optResolve(QStringList opt){
  /*
   * resolve URL and print
   * TODO: arguments
   */

  QString repo;
  bool rndRepo=false;
/*
  repo = arguments.at(2);

  if (arguments.count() >= 3){
    //qout << (arguments.at(3).compare("false")||arguments.at(3).compare("0"));
    qout << (arguments.at(3).compare("false"));
  }

  qout << ssu.repoUrl(arguments.at(2));
  QCoreApplication::exit(1);
*/

}

void RndSsuCli::optStatus(){
  QTextStream qout(stdout);
  SsuDeviceInfo deviceInfo;

  /*
   * print device information and registration status
   */
  qout << "Device registration status: "
       << (ssu.isRegistered() ? "registered" : "not registered") << endl;
  qout << "Device family: " << deviceInfo.deviceFamily() << endl;
  qout << "Device model: " << deviceInfo.deviceModel() << endl;
  qout << "Device variant: " << deviceInfo.deviceVariant() << endl;
  qout << "Device UID: " << deviceInfo.deviceUid() << endl;
}

void RndSsuCli::optUpdateCredentials(QStringList opt){
  QTextStream qout(stdout);
  /*
   * update the credentials
   * optional argument: -f
   */
  bool force=false;
  if (opt.count() == 3 && opt.at(2) == "-f")
    force=true;

  if (!ssu.isRegistered()){
      qout << "Device is not registered, can't update credentials" << endl;
      state = Idle;
      QCoreApplication::exit(1);
  } else {
    ssu.updateCredentials(force);
    state = Busy;
  }
}

void RndSsuCli::optUpdateRepos(){
  SsuRepoManager repoManager;
  repoManager.update();
}

void RndSsuCli::run(){
  QTextStream qout(stdout);

  QStringList arguments = QCoreApplication::arguments();

  if (arguments.at(0).endsWith("rndssu"))
    qout << "NOTE: this binary is now called ssu. The rndssu symlink will go away after some time" << endl;

  // make sure there's a first argument to parse
  if (arguments.count() < 2){
    usage();
    return;
  }

  // everything without additional arguments gets handled here
  // functions with arguments need to take care of argument validation themselves
  if (arguments.count() == 2){
    if (arguments.at(1) == "register" || arguments.at(1) == "r")
      optRegister();
    else if (arguments.at(1) == "status" || arguments.at(1) == "s")
      optStatus();
    else if (arguments.at(1) == "updaterepos" || arguments.at(1) == "ur")
      optUpdateRepos();
    else
      state = UserError;
  } else if (arguments.count() >= 3){
    if (arguments.at(1) == "addrepo" || arguments.at(1) == "ar")
      optModifyRepo(Add, arguments);
    else if (arguments.at(1) == "removerepo" || arguments.at(1) == "rr")
      optModifyRepo(Remove, arguments);
    else if (arguments.at(1) == "enablerepo" || arguments.at(1) == "er")
      optModifyRepo(Enable, arguments);
    else if (arguments.at(1) == "disablerepo" || arguments.at(1) == "dr")
      optModifyRepo(Disable, arguments);
    else if (arguments.at(1) == "resolve")
      optResolve(arguments);
    else
      state = UserError;
  } else
    state = UserError;

  // functions accepting 0 or more arguments; those need to set state to Idle
  // on success
  if (arguments.at(1) == "repos" || arguments.at(1) == "lr")
    optRepos(arguments);
  else if (arguments.at(1) == "flavour" || arguments.at(1) == "fl")
    optFlavour(arguments);
  else if (arguments.at(1) == "mode" || arguments.at(1) == "m")
    optMode(arguments);
  else if (arguments.at(1) == "release" || arguments.at(1) == "re")
    optRelease(arguments);
  else if (arguments.at(1) == "update" || arguments.at(1) == "up")
    optUpdateCredentials(arguments);

  // functions that need to wait for a response from ssu should set a flag so
  // we can do default exit catchall here
  if (state == Idle)
    QCoreApplication::exit(0);
  else if (state == UserError)
    usage();
}

void RndSsuCli::usage(){
  QTextStream qout(stdout);
  qout << "\nUsage: ssu <command> [-command-options] [arguments]" << endl
       << endl
       << "Repository management:" << endl
       << "\tupdaterepos, ur        \tupdate repository files" << endl
       << "\trepos, lr              \tlist configured repositories" << endl
       << "\t           [-m]        \tformat output suitable for kickstart" << endl
       << "\t           [device]    \tuse repos for 'device'" << endl
       << "\t           [flags]     \tadditional flags" << endl
       << "\t           rnd=<true|false> \tset rnd or release mode (default: take from host)" << endl
       << "\taddrepo, ar <repo>     \tadd this repository" << endl
       << "\t           [url]       \tspecify URL, if not configured" << endl
       << "\tremoverepo, rr <repo>  \tremove this repository from configuration" << endl
       << "\tenablerepo, er <repo>  \tenable this repository" << endl
       << "\tdisablerepo, dr <repo> \tdisable this repository" << endl
       << endl
       << "Configuration management:" << endl
       << "\tflavour, fl     \tdisplay flavour used (RnD only)" << endl
       << "\t  [newflavour]  \tset new flavour" << endl
       << "\trelease, re     \tdisplay release used" << endl
       << "\t  [-r]          \tuse RnD release" << endl
       << "\t  [newrelease]  \tset new (RnD)release" << endl
       << "\tmode, m         \tdisplay current device mode" << endl
       << "\t  [newmode]     \tset new device mode" << endl
       << endl
       << "Device management:" << endl
       << "\tstatus, s     \tprint registration status and device information" << endl
       << "\tregister, r   \tregister this device" << endl
       << "\tupdate, up    \tupdate repository credentials" << endl
       << "\t      [-f]    \tforce update" << endl
       << endl;
  qout.flush();
  QCoreApplication::exit(1);
}
