/**
 * @file ssud.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include "ssud.h"
#include "ssuadaptor.h"

#include "libssu/ssudeviceinfo.h"
#include "libssu/ssurepomanager.h"

#include <QDBusConnection>

const char *Ssud::SERVICE_NAME = "org.nemo.ssu";
const char *Ssud::OBJECT_PATH = "/org/nemo/ssu";

Ssud::Ssud(QObject *parent): QObject(parent){
  QDBusConnection connection = QDBusConnection::systemBus();
  if (!connection.registerService(SERVICE_NAME)) {
    qFatal("Cannot register D-Bus service at %s", SERVICE_NAME);
  }

  if (!connection.registerObject(OBJECT_PATH, this)) {
    qFatal("Cannot register object at %s", OBJECT_PATH);
  }

  new SsuAdaptor(this);

  connect(&ssu, SIGNAL(done()),
          this, SIGNAL(done()));
  connect(&ssu, SIGNAL(credentialsChanged()),
          this, SIGNAL(credentialsChanged()));
  connect(&ssu, SIGNAL(registrationStatusChanged()),
          this, SIGNAL(registrationStatusChanged()));
}

Ssud::~Ssud(){
}

QString Ssud::deviceModel(){
  SsuDeviceInfo deviceInfo;

  return deviceInfo.deviceModel();
}

QString Ssud::deviceFamily(){
  SsuDeviceInfo deviceInfo;

  return deviceInfo.deviceFamily();
}

QString Ssud::deviceUid(){
  SsuDeviceInfo deviceInfo;

  return deviceInfo.deviceUid();
}

QString Ssud::deviceVariant(){
  SsuDeviceInfo deviceInfo;

  return deviceInfo.deviceVariant();
}

bool Ssud::error(){
  return ssu.error();
}

QString Ssud::lastError(){
  return ssu.lastError();
}

void Ssud::quit(){
  QCoreApplication::quit();
}

bool Ssud::isRegistered(){
  return ssu.isRegistered();
}

void Ssud::registerDevice(const QString &username, const QString &password){
  ssu.sendRegistration(username, password);
}

void Ssud::unregisterDevice(){
  ssu.unregister();
};


int Ssud::deviceMode(){
  return ssu.deviceMode();
}

void Ssud::setDeviceMode(int mode){
  ssu.setDeviceMode(mode);

  SsuRepoManager repoManager;
  repoManager.update();
}

QString Ssud::flavour(){
  return ssu.flavour();
}

void Ssud::setFlavour(const QString &flavour){
  ssu.setFlavour(flavour);

  SsuRepoManager repoManager;
  repoManager.update();
}


QString Ssud::release(bool rnd){
  return ssu.release(rnd);
}

void Ssud::setRelease(const QString &release, bool rnd){
  ssu.setRelease(release, rnd);

  SsuRepoManager repoManager;
  repoManager.update();
}

void Ssud::modifyRepo(int action, const QString &repo){
  SsuRepoManager repoManager;

  switch(action){
    case Add:
      repoManager.add(repo);
      break;
    case Remove:
      repoManager.remove(repo);
      break;
    case Disable:
      repoManager.disable(repo);
      break;
    case Enable:
      repoManager.enable(repo);
      break;
  }

  repoManager.update();
}

void Ssud::addRepo(const QString &repo, const QString &url){
  SsuRepoManager repoManager;
  repoManager.add(repo, url);
  repoManager.update();
}

void Ssud::updateRepos(){
  SsuRepoManager repoManager;
  repoManager.update();
}
