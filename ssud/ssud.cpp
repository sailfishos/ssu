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
  if (!connection.registerObject(OBJECT_PATH, this)) {
    qFatal("Cannot register object at %s", OBJECT_PATH);
  }

  if (!connection.registerService(SERVICE_NAME)) {
    qFatal("Cannot register D-Bus service at %s", SERVICE_NAME);
  }

  // prepare for controlled suicide on boredom
  const int AUTOCLOSE_TIMEOUT_MS = 180 * 1000;

  autoclose.setSingleShot(true);
  autoclose.setInterval(AUTOCLOSE_TIMEOUT_MS);

  connect(&autoclose, SIGNAL(timeout()),
          this, SLOT(quit()));

  new SsuAdaptor(this);

  connect(&ssu, SIGNAL(done()),
          this, SIGNAL(done()));
  connect(&ssu, SIGNAL(credentialsChanged()),
          this, SIGNAL(credentialsChanged()));
  connect(&ssu, SIGNAL(registrationStatusChanged()),
          this, SIGNAL(registrationStatusChanged()));

  // a cry for help everytime we do something to prevent suicide
  autoclose.start();
}

Ssud::~Ssud(){
}

QString Ssud::deviceModel(){
  SsuDeviceInfo deviceInfo;

  autoclose.start();
  return deviceInfo.deviceModel();
}

QString Ssud::deviceFamily(){
  SsuDeviceInfo deviceInfo;

  autoclose.start();
  return deviceInfo.deviceFamily();
}

QString Ssud::deviceUid(){
  SsuDeviceInfo deviceInfo;

  autoclose.start();
  return deviceInfo.deviceUid();
}

QString Ssud::deviceVariant(){
  SsuDeviceInfo deviceInfo;

  autoclose.start();
  return deviceInfo.deviceVariant();
}

QString Ssud::displayName(int type){
  SsuDeviceInfo deviceInfo;

  autoclose.start();
  return deviceInfo.displayName(type);
}

bool Ssud::error(){
  autoclose.start();
  return ssu.error();
}

QString Ssud::lastError(){
  autoclose.start();
  return ssu.lastError();
}

void Ssud::quit(){
  QCoreApplication::quit();
}

bool Ssud::isRegistered(){
  autoclose.start();
  return ssu.isRegistered();
}

void Ssud::registerDevice(const QString &username, const QString &password){
  autoclose.stop();
  ssu.sendRegistration(username, password);
  autoclose.start();
}

void Ssud::unregisterDevice(){
  autoclose.start();
  ssu.unregister();
};

QString Ssud::domain(){
    autoclose.start();
    return ssu.domain();
}

Ssu::DeviceModeFlags Ssud::deviceMode(){
  autoclose.start();
  return ssu.deviceMode();
}

void Ssud::setDeviceMode(int mode){
  setDeviceMode(mode, Ssu::Replace);
}

void Ssud::setDeviceMode(int mode, int editMode){
  ssu.setDeviceMode(
    Ssu::DeviceModeFlags(mode),
    Ssu::EditMode(editMode)
    );

  SsuRepoManager repoManager;
  repoManager.update();
  autoclose.start();
}

QString Ssud::flavour(){
  autoclose.start();
  return ssu.flavour();
}

void Ssud::setFlavour(const QString &flavour){
  ssu.setFlavour(flavour);

  SsuRepoManager repoManager;
  repoManager.update();
  autoclose.start();
}


QString Ssud::release(bool rnd){
  autoclose.start();
  return ssu.release(rnd);
}

void Ssud::setRelease(const QString &release, bool rnd){
  ssu.setRelease(release, rnd);

  SsuRepoManager repoManager;
  repoManager.update();
  autoclose.start();
}

void Ssud::modifyRepo(int action, const QString &repo){
  SsuRepoManager repoManager;

  autoclose.stop();

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
  autoclose.start();
}

void Ssud::addRepo(const QString &repo, const QString &url){
  SsuRepoManager repoManager;
  repoManager.add(repo, url);
  repoManager.update();
  autoclose.start();
}

void Ssud::updateRepos(){
  SsuRepoManager repoManager;
  autoclose.stop();
  repoManager.update();
  autoclose.start();
}
