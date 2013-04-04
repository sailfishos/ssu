/**
 * @file rndssuclitest.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#include "rndssuclitest.h"

#include <stdlib.h>
#include <zypp/media/UrlResolverPlugin.h>

#include <QtTest/QtTest>

#include "libssu/sandbox_p.h"
#include "testutils/process.h"

typedef QStringList Args; // improve readability

void RndSsuCliTest::init(){
  Q_ASSERT(m_sandbox == 0);

  m_sandbox = new Sandbox(QString("%1/configroot").arg(TESTS_DATA_PATH),
      Sandbox::UseAsSkeleton, Sandbox::ChildProcesses);
  setenv("LD_PRELOAD", qPrintable(QString("%1/libsandboxhook.so").arg(TESTS_PATH)), 1);
}

void RndSsuCliTest::cleanup(){
  delete m_sandbox;
  m_sandbox = 0;
}

void RndSsuCliTest::testSubcommandFlavour(){
  Process ssu;
  QString output;

  // set flavour to 'release'
  ssu.execute("ssu", Args() << "flavour" << "release");
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  output = ssu.execute("ssu", Args() << "flavour").trimmed();
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  QCOMPARE(output, QString("Device flavour is currently: release"));

  // set flavour to 'testing'
  ssu.execute("ssu", Args() << "flavour" << "testing");
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  output = ssu.execute("ssu", Args() << "flavour").trimmed();
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  QCOMPARE(output, QString("Device flavour is currently: testing"));
}

void RndSsuCliTest::testSubcommandRelease(){
  Process ssu;
  QString output;

  // set release to latest
  ssu.execute("ssu", Args() << "release" << "latest");
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  output = ssu.execute("ssu", Args() << "release").trimmed();
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  QCOMPARE(output, QString("Device release is currently: latest"));

  // set release to next
  ssu.execute("ssu", Args() << "release" << "next");
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  output = ssu.execute("ssu", Args() << "release").trimmed();
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  QCOMPARE(output, QString("Device release is currently: next"));

  // verify the mode is release
  output = ssu.execute("ssu", Args() << "mode").trimmed();
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  QCOMPARE(output, QString("Device mode is: 0 ()"));

  // set RnD release to latest
  ssu.execute("ssu", Args() << "release" << "-r" << "latest");
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  output = ssu.execute("ssu", Args() << "release" << "-r").trimmed();
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  QCOMPARE(output, QString("Device release (RnD) is currently: latest"));

  // set RnD release to next
  ssu.execute("ssu", Args() << "release" << "-r" << "next");
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  output = ssu.execute("ssu", Args() << "release" << "-r").trimmed();
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  QCOMPARE(output, QString("Device release (RnD) is currently: next"));

  // verify the mode is RnD
  output = ssu.execute("ssu", Args() << "mode").trimmed();
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  QCOMPARE(output, QString("Device mode is: 2 (RndMode)"));
}

void RndSsuCliTest::testSubcommandMode(){
  Process ssu;
  QString output;

  // set release mode
  ssu.execute("ssu", Args() << "mode" << "0");
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  output = ssu.execute("ssu", Args() << "mode").trimmed();
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  QCOMPARE(output, QString("Device mode is: 0 ()"));

  // set RnD mode
  ssu.execute("ssu", Args() << "mode" << "2");
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  output = ssu.execute("ssu", Args() << "mode").trimmed();
  QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));

  QCOMPARE(output, QString("Device mode is: 2 (RndMode)"));
}
