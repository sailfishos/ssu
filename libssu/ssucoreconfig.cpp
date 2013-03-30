/**
 * @file ssucoreconfig.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include <QFile>
#include <QTextStream>

#include "ssucoreconfig.h"
#include "ssulog.h"

SsuCoreConfig *SsuCoreConfig::ssuCoreConfig = 0;

SsuCoreConfig *SsuCoreConfig::instance(){
  if (!ssuCoreConfig)
    ssuCoreConfig = new SsuCoreConfig();

  return ssuCoreConfig;
}

QPair<QString, QString> SsuCoreConfig::credentials(QString scope){
  QPair<QString, QString> ret;
  beginGroup("credentials-" + scope);
  ret.first = value("username").toString();
  ret.second = value("password").toString();
  endGroup();
  return ret;
}

QString SsuCoreConfig::credentialsScope(QString repoName, bool rndRepo){
  if (contains("credentials-scope"))
    return value("credentials-scope").toString();
  else
    return "your-configuration-is-broken-and-does-not-contain-credentials-scope";
}

QString SsuCoreConfig::credentialsUrl(QString scope){
  if (contains("credentials-url-" + scope))
    return value("credentials-url-" + scope).toString();
  else
    return "your-configuration-is-broken-and-does-not-contain-credentials-url-for-" + scope;
}

QString SsuCoreConfig::flavour(){
  if (contains("flavour"))
    return value("flavour").toString();
  else
    return "release";
}

int SsuCoreConfig::deviceMode(){
  if (!contains("deviceMode"))
    return Ssu::ReleaseMode;
  else
    return value("deviceMode").toInt();
}

QString SsuCoreConfig::domain(){
  if (contains("domain"))
    return value("domain").toString();
  else
    return "";
}

bool SsuCoreConfig::isRegistered(){
  if (!contains("privateKey"))
    return false;
  if (!contains("certificate"))
    return false;
  return value("registered").toBool();
}

QDateTime SsuCoreConfig::lastCredentialsUpdate(){
  return value("lastCredentialsUpdate").toDateTime();
}

QString SsuCoreConfig::release(bool rnd){
  if (rnd)
    return value("rndRelease").toString();
  else
    return value("release").toString();
}

void SsuCoreConfig::setDeviceMode(int mode, int editMode){
  int oldMode = value("deviceMode").toInt();

  if ((editMode & Ssu::Add) == Ssu::Add){
    oldMode |= mode;
  } else if ((editMode & Ssu::Remove) == Ssu::Remove){
    oldMode &= ~mode;
  } else
    oldMode = mode;

  setValue("deviceMode", oldMode);
  sync();
}

void SsuCoreConfig::setFlavour(QString flavour){
  setValue("flavour", flavour);
  // flavour is RnD only, so enable RnD mode
  setDeviceMode(Ssu::RndMode, Ssu::Add);
  sync();
}

void SsuCoreConfig::setRelease(QString release, bool rnd){
  if (rnd) {
    setValue("rndRelease", release);
    // switch rndMode on/off when setting releases
    setDeviceMode(Ssu::RndMode, Ssu::Add);
  } else {
    setValue("release", release);
    setDeviceMode(Ssu::RndMode, Ssu::Remove);
  }
  sync();
}

void SsuCoreConfig::setDomain(QString domain){
  setValue("domain", domain);
  sync();
}

bool SsuCoreConfig::useSslVerify(){
  if (contains("ssl-verify"))
    return value("ssl-verify").toBool();
  else
    return true;
}
