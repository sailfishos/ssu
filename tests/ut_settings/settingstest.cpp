/**
 * @file settingstest.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#include "settingstest.h"

#include <QtTest/QtTest>

#include <ssusettings.h>

void SettingsTest::initTestCase(){

}

void SettingsTest::cleanupTestCase(){

}

void SettingsTest::testMerge_data(){
  // Key names state which file(s) a key is set in (master, foo or bar). Value
  // is always in form "<file>-value" based on which file it is taken from.

  const QScopedPointer<QTemporaryFile> masterFile(
    QTemporaryFile::createLocalFile(":/testdata/merge/settings.ini"));
  QSettings master(masterFile->fileName(), QSettings::IniFormat);

  const QStringList settingsFiles = QStringList()
    << ":/testdata/merge/settings.d/bar.ini"
    << ":/testdata/merge/settings.d/foo.ini";

  SsuSettings::merge(&master, settingsFiles);

  QSettings expected(":/testdata/merge/merged.ini", QSettings::IniFormat);

  const QSet<QString> masterKeys = master.allKeys().toSet();
  const QSet<QString> expectedKeys = expected.allKeys().toSet();

  QTest::addColumn<bool>("keyIsMerged");
  QTest::addColumn<bool>("keyShouldBeMerged");
  QTest::addColumn<QString>("actualValue");
  QTest::addColumn<QString>("expectedValue");

  foreach (const QString &key, masterKeys + expectedKeys){
    QTest::newRow(qPrintable(key))
      << masterKeys.contains(key)
      << expectedKeys.contains(key)
      << master.value(key).toString()
      << expected.value(key).toString();
  }
}

void SettingsTest::testMerge(){
  QFETCH(bool, keyIsMerged);
  QFETCH(bool, keyShouldBeMerged);
  QFETCH(QString, actualValue);
  QFETCH(QString, expectedValue);

  QCOMPARE(keyIsMerged, keyShouldBeMerged);
  QCOMPARE(actualValue, expectedValue);
}
