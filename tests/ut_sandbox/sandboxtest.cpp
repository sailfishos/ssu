/**
 * @file sandboxtest.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#include "sandboxtest.h"

#include <QtTest/QtTest>

#include "libssu/sandbox_p.h"

void SandboxTest::test(){

  const QDir::Filters noHidden = QDir::AllEntries | QDir::NoDotAndDotDot;

  QCOMPARE(QDir(TESTS_DATA_PATH "/world").entryList(noHidden, QDir::Name),
      QStringList()
      << "world-and-sandbox"
      << "world-only"
      << "world-only-to-be-copied-into-sandbox");

  QVERIFY(!QFileInfo(TESTS_DATA_PATH "/world/world-only").isWritable());
  QCOMPARE(readAll(TESTS_DATA_PATH "/world/world-only").trimmed(),
      QString("world/world-only"));

  QVERIFY(!QFileInfo(TESTS_DATA_PATH "/world/world-and-sandbox").isWritable());
  QCOMPARE(readAll(TESTS_DATA_PATH "/world/world-and-sandbox").trimmed(),
      QString("world/world-and-sandbox"));

  QVERIFY(!QFileInfo(TESTS_DATA_PATH "/world/world-only-to-be-copied-into-sandbox").isWritable());
  QCOMPARE(readAll(TESTS_DATA_PATH "/world/world-only-to-be-copied-into-sandbox").trimmed(),
      QString("world/world-only-to-be-copied-into-sandbox"));

  QVERIFY(!QFileInfo(TESTS_DATA_PATH "/world/sandbox-only").exists());


  Sandbox sandbox(TESTS_DATA_PATH "/sandbox",
      Sandbox::UseAsSkeleton, Sandbox::ThisProcess | Sandbox::ChildProcesses);
  sandbox.addWorldFiles(TESTS_DATA_PATH "/world", QDir::AllEntries,
      QStringList() << "*-to-be-copied-into-sandbox");
  QVERIFY(sandbox.activate());


  QCOMPARE(QDir(TESTS_DATA_PATH "/world").entryList(noHidden, QDir::Name),
      QStringList()
      << "world-and-sandbox"
      << "world-only"
      << "world-only-to-be-copied-into-sandbox");

  QVERIFY(!QFileInfo(TESTS_DATA_PATH "/world/world-only").isWritable());
  QCOMPARE(readAll(TESTS_DATA_PATH "/world/world-only").trimmed(),
      QString("world/world-only"));

  QVERIFY(QFileInfo(TESTS_DATA_PATH "/world/world-and-sandbox").isWritable());
  QCOMPARE(readAll(TESTS_DATA_PATH "/world/world-and-sandbox").trimmed(),
      QString("sandbox/world-and-sandbox"));

  QVERIFY(QFileInfo(TESTS_DATA_PATH "/world/world-only-to-be-copied-into-sandbox").isWritable());
  QCOMPARE(readAll(TESTS_DATA_PATH "/world/world-only-to-be-copied-into-sandbox").trimmed(),
      QString("world/world-only-to-be-copied-into-sandbox"));

  QVERIFY(QFileInfo(TESTS_DATA_PATH "/world/sandbox-only").exists());
  QVERIFY(QFileInfo(TESTS_DATA_PATH "/world/sandbox-only").isWritable());
  QCOMPARE(readAll(TESTS_DATA_PATH "/world/sandbox-only").trimmed(),
      QString("sandbox/sandbox-only"));
}

QString SandboxTest::readAll(const QString &fileName){
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)){
    qWarning("%s: Failed to open file for reading: '%s': %s", Q_FUNC_INFO, qPrintable(fileName),
        qPrintable(file.errorString()));
    return QString();
  }

  return file.readAll();
}
