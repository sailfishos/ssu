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
  if (!m_sandbox->activate()){
    QFAIL("Failed to activate sandbox");
  }
  setenv("LD_PRELOAD", qPrintable(QString("%1/libsandboxhook.so").arg(TESTS_PATH)), 1);

  m_bus = new QProcess(this);
  m_bus->start("dbus-daemon",
      Args() << "--session" << "--nofork" << "--nopidfile" << "--print-address");
  if (!m_bus->waitForReadyRead()){
    QFAIL("Failed to run sandboxed D-Bus instance");
  }
  const QByteArray busAddress = m_bus->readAllStandardOutput();
  setenv("DBUS_SESSION_BUS_ADDRESS", busAddress.constData(), 1);
  setenv("DBUS_SYSTEM_BUS_ADDRESS", busAddress.constData(), 1);

  m_ssud = new QProcess(this);
  m_ssud->start("ssud");
  if (!m_ssud->waitForStarted()){
    QFAIL("Failed to run sandboxed ssud instance");
  }
}

void RndSsuCliTest::cleanup(){
  if (m_ssud->state() != QProcess::Running){
    QFAIL("Sandboxed ssud instance exited unexpectedly");
  }
  delete m_ssud;
  m_ssud = 0;

  if (m_bus->state() != QProcess::Running){
    QFAIL("Sandboxed D-Bus instance exited unexpectedly");
  }
  unsetenv("DBUS_SESSION_BUS_ADDRESS");
  unsetenv("DBUS_SYSTEM_BUS_ADDRESS");
  delete m_bus;
  m_bus = 0;

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
