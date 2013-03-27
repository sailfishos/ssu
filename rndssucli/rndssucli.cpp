/**
 * @file rndssucli.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#include <QCoreApplication>

#include <termios.h>

#include <ssudeviceinfo.h>

#include "rndssucli.h"

RndSsuCli::RndSsuCli(): QObject(){
  connect(this,SIGNAL(done()),
          QCoreApplication::instance(),SLOT(quit()), Qt::DirectConnection);
  connect(&ssu, SIGNAL(done()),
          this, SLOT(handleResponse()));
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


void RndSsuCli::optFlavour(QString newFlavour){
  QTextStream qout(stdout);

  if (newFlavour != ""){
    qout << "Changing flavour from " << ssu.flavour()
         << " to " << newFlavour << endl;
    ssu.setFlavour(newFlavour);
  } else
    qout << "Device flavour is currently: " << ssu.flavour() << endl;

  QCoreApplication::exit(0);
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
}

void RndSsuCli::optResolve(){
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

  QCoreApplication::exit(1);
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

  QCoreApplication::exit(0);
}

void RndSsuCli::optUpdate(bool force){
  QTextStream qout(stdout);
  /*
   * update the credentials
   * optional argument: -f
   */
  if (!ssu.isRegistered()){
      qout << "Device is not registered, can't update credentials" << endl;
      QCoreApplication::exit(1);
  } else
    ssu.updateCredentials(force);

  QCoreApplication::exit(0);
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

  if (arguments.at(1) == "register" && arguments.count() == 2){
    optRegister();
  } else if (arguments.at(1) == "flavour" &&
             (arguments.count() == 2 || arguments.count() == 3)){
    if (arguments.count() == 2) optFlavour();
    else optFlavour(arguments.at(2));
  } else if (arguments.at(1) == "update" &&
             (arguments.count() == 2 || arguments.count() == 3)){
    if (arguments.count() == 3 && arguments.at(2) == "-f")
      optUpdate(true);
    else optUpdate(false);
  } else if (arguments.at(1) == "resolve"){
    optResolve();
  } else if (arguments.at(1) == "status" && arguments.count() == 2){
    optStatus();
  } else
    usage();
}

void RndSsuCli::usage(){
  QTextStream qout(stdout);
  qout << "Usage: rndssu flavour [flavour]|register|update [-f]|status" << endl;
  QCoreApplication::exit(1);
}
