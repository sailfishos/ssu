/**
 * @file coreconfigtest.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#include "coreconfigtest.h"

#include <QtTest/QtTest>

#include "libssu/ssucoreconfig.h"

void CoreconfigTest::testCredentialsScope(){
  QCOMPARE(SsuCoreConfig::instance()->credentialsScope("/*ignored*/", false), QString("example"));

}

void CoreconfigTest::testCredentials(){
  QCOMPARE(SsuCoreConfig::instance()->credentials("example").first, QString("example_username"));
  QCOMPARE(SsuCoreConfig::instance()->credentials("example").second, QString("example_password"));
}

void CoreconfigTest::testCredentialsUrl(){
  QCOMPARE(SsuCoreConfig::instance()->credentialsUrl("example"), QString("http://creden.tia.ls/"));
}

void CoreconfigTest::testFlavour(){
  QCOMPARE(SsuCoreConfig::instance()->flavour(), QString("testing"));
  SsuCoreConfig::instance()->setFlavour("release");
  QCOMPARE(SsuCoreConfig::instance()->flavour(), QString("release"));
}

void CoreconfigTest::testDeviceMode(){
  SsuCoreConfig::instance()->remove("deviceMode");
  QCOMPARE(SsuCoreConfig::instance()->deviceMode(), (int)Ssu::ReleaseMode);
  SsuCoreConfig::instance()->setDeviceMode(Ssu::ReleaseMode, Ssu::Add);
  QCOMPARE(SsuCoreConfig::instance()->deviceMode(), (int)Ssu::ReleaseMode);
  SsuCoreConfig::instance()->setDeviceMode(Ssu::LenientMode, Ssu::Add);
  QCOMPARE(SsuCoreConfig::instance()->deviceMode(), (int)Ssu::ReleaseMode | Ssu::LenientMode);
  SsuCoreConfig::instance()->setDeviceMode(Ssu::ReleaseMode, Ssu::Remove);
  QCOMPARE(SsuCoreConfig::instance()->deviceMode(), (int)Ssu::LenientMode);
  SsuCoreConfig::instance()->setDeviceMode(Ssu::ReleaseMode, Ssu::Replace);
  QCOMPARE(SsuCoreConfig::instance()->deviceMode(), (int)Ssu::ReleaseMode);
}

void CoreconfigTest::testDomain(){
  QCOMPARE(SsuCoreConfig::instance()->domain(), QString(""));
  SsuCoreConfig::instance()->setDomain("foo");
  QCOMPARE(SsuCoreConfig::instance()->domain(), QString("foo"));
}

void CoreconfigTest::testRegistered(){
  QCOMPARE(SsuCoreConfig::instance()->isRegistered(), false);
  SsuCoreConfig::instance()->setValue("registered", true);
  QCOMPARE(SsuCoreConfig::instance()->isRegistered(), false);
  SsuCoreConfig::instance()->setValue("privateKey", "fooKey");
  QCOMPARE(SsuCoreConfig::instance()->isRegistered(), false);
  SsuCoreConfig::instance()->setValue("certificate", "fooCert");
  QCOMPARE(SsuCoreConfig::instance()->isRegistered(), true);
}

void CoreconfigTest::testLastCredentialsUpdate(){
  QCOMPARE(SsuCoreConfig::instance()->lastCredentialsUpdate(), QDateTime());
  SsuCoreConfig::instance()->setValue("lastCredentialsUpdate",
      QDateTime::fromString("2013-04-08", Qt::ISODate));
  QCOMPARE(SsuCoreConfig::instance()->lastCredentialsUpdate(),
      QDateTime::fromString("2013-04-08", Qt::ISODate));
}

void CoreconfigTest::testRelease(){
  QCOMPARE(SsuCoreConfig::instance()->release(false), QString("latest"));
  QCOMPARE(SsuCoreConfig::instance()->release(true), QString("next"));
  SsuCoreConfig::instance()->setRelease("next", false);
  QCOMPARE(SsuCoreConfig::instance()->release(false), QString("next"));
  QCOMPARE(SsuCoreConfig::instance()->release(true), QString("next"));
  SsuCoreConfig::instance()->setRelease("latest", true);
  QCOMPARE(SsuCoreConfig::instance()->release(false), QString("next"));
  QCOMPARE(SsuCoreConfig::instance()->release(true), QString("latest"));
}

void CoreconfigTest::testSslVerify(){
  QCOMPARE(SsuCoreConfig::instance()->useSslVerify(), true);
  SsuCoreConfig::instance()->setValue("ssl-verify", false);
  QCOMPARE(SsuCoreConfig::instance()->useSslVerify(), false);
}
