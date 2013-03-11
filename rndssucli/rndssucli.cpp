/**
 * @file rndssucli.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#include <QCoreApplication>
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


void RndSsuCli::run(){
  QTextStream qout(stdout);

  QStringList arguments = QCoreApplication::arguments();

  if (arguments.count() >= 2){
    usage();
    return;
  }

  if (arguments.at(1) == "register"){
    QString username, password;
    QTextStream qin(stdin);

    qout << "Username: " << flush;
    username = qin.readLine();
    qout << "Password: " << flush;
    password = qin.readLine();

    ssu.sendRegistration(username, password);
  } else if (arguments.at(1) == "update"){
    if (!ssu.isRegistered()){
      qout << "Device is not registered, can't update credentials" << endl;
      QCoreApplication::exit(1);
    } else {
      ssu.updateCredentials();
    }
  } else if (arguments.at(1) == "resolve"){
    QString repo;
    bool rndRepo=false;
    if (arguments.count() <= 2){
      usage();
      return;
    }
    repo = arguments.at(2);

    if (arguments.count() >= 3){
      //qout << (arguments.at(3).compare("false")||arguments.at(3).compare("0"));
      qout << (arguments.at(3).compare("false"));

    }

    qout << ssu.repoUrl(arguments.at(2));
    QCoreApplication::exit(1);
  } else if (arguments.at(1) == "status"){
    qout << "Device registration status: "
         << (ssu.isRegistered() ? "registered" : "not registered") << endl;
    qout << "Device family: " << ssu.deviceFamily() << endl;
    qout << "Device model: " << ssu.deviceModel() << endl;
    qout << "Device UID: " << ssu.deviceUid() << endl;
    QCoreApplication::exit(1);
  } else
    usage();

}

void RndSsuCli::usage(){
  QTextStream qout(stdout);
  qout << "Usage: rndssu register|update|status" << endl;
  QCoreApplication::exit(1);
}
