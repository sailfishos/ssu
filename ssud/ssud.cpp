/**
 * @file ssud.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include "ssud.h"
#include "ssuadaptor.h"

#include "libssu/ssudeviceinfo.h"

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
};

QString Ssud::deviceUid(){
  SsuDeviceInfo deviceInfo;

  return deviceInfo.deviceUid();
};

bool Ssud::error(){
  return ssu.error();
}

QString Ssud::lastError(){
  return ssu.lastError();
}

bool Ssud::isRegistered(){
  return ssu.isRegistered();
}

void Ssud::quit(){
  QCoreApplication::quit();
}

void Ssud::registerDevice(const QString &username, const QString &password){
  ssu.sendRegistration(username, password);
}

void Ssud::unregisterDevice(){
  ssu.unregister();
};
