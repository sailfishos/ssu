/**
 * @file rndssucli.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#include <QCoreApplication>

#include <termios.h>

#include <ssudeviceinfo.h>
#include <ssurepomanager.h>

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
  qout << "Mode handling is currently not implemented" << endl;
  state = Idle;
}

void RndSsuCli::optModifyRepo(int action, QStringList opt){
  SsuRepoManager repoManager;

  QTextStream qout(stdout);
  qout << "Repository management is currently not implemented" << endl;

  if (opt.count() == 3){

  } else if (opt.count() == 4){

  }
/*
  switch(action){
    case Add:
    case Remove:
    case Disable:
    case Enable:
  }
*/
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
      ssu.setRelease(opt.at(2));
      state = Idle;
    }
  } else if (opt.count() == 2) {
    qout << "Device release is currently: " << ssu.release() << endl;
    state = Idle;
  } else if (opt.count() == 4 && opt.at(2) == "-r"){
    qout << "Changing release (RnD) from " << ssu.release(true)
         << " to " << opt.at(3) << endl;
    ssu.setRelease(opt.at(3), true);
    state = Idle;
  }
}

void RndSsuCli::optRepos(QStringList opt){
  SsuRepoManager repoManager;
  SsuDeviceInfo deviceInfo;
  QHash<QString, QString> repoParameters;
  bool rndRepo = false;

  QHash<QString, QString> repoOverride;
  repoOverride.insert("release", "@RELEASE@");
  repoOverride.insert("rndRelease", "@RNDRELEASE@");
  repoOverride.insert("flavour", "@FLAVOUR@");
  repoOverride.insert("arch", "@ARCH@");
  //repoOverride.insert("", "");

  QStringList repos = deviceInfo.repos(rndRepo);

  // for each repository, print repo and resolve url
  foreach (const QString &repo, repos){
    QString repoUrl = ssu.repoUrl(repo, rndRepo, repoParameters, repoOverride);
    qDebug() << "Resolved to " << repoUrl;
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
