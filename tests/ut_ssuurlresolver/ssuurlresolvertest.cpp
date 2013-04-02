/**
 * @file ssuurlresolvertest.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#include "ssuurlresolvertest.h"

#include <stdlib.h>
#include <zypp/media/UrlResolverPlugin.h>

#include <QtTest/QtTest>

/**
 * @class SsuUrlResolverTest
 * @brief Tests libzypp UrlResolverPlugin plugin compatibility
 *
 * This test verifies the UrlResolverPlugin works well with installed version of libzypp.
 */

void SsuUrlResolverTest::initTestCase(){
  setenv("LD_PRELOAD", qPrintable(QString("%1/libsandboxhook.so").arg(TESTS_PATH)), 1);
  setenv("SSU_TESTS_SANDBOX", qPrintable(QString("%1/configroot").arg(TESTS_DATA_PATH)), 1);
}

void SsuUrlResolverTest::test_data(){
  QTest::addColumn<QString>("input");
  QTest::addColumn<QString>("expected");

  QTest::newRow("basic")
    << "plugin:ssu?repo=mer-core&debug&arch=i586"
    << "https://packages.testing.com//mer/i586/debug/?credentials=example";
}

void SsuUrlResolverTest::test(){
  QFETCH(QString, input);
  QFETCH(QString, expected);

  zypp::media::UrlResolverPlugin::HeaderList customHeaders;
  const QString resolved = QString::fromStdString(
      zypp::media::UrlResolverPlugin::resolveUrl(input.toStdString(), customHeaders).asString());

  QCOMPARE(resolved, expected);
}
