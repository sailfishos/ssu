/**
 * @file settingstest.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#include "settingstest.h"

#include <QtTest/QtTest>

#include "libssu/ssusettings.h"
#include "upgradetesthelper.h"

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

void SettingsTest::testUpgrade_data(){
  // Read recipe
  QFile recipe(":/testdata/upgrade/recipe");
  QVERIFY(recipe.open(QIODevice::ReadOnly));
  QList<UpgradeTestHelper::TestCase> testCases = UpgradeTestHelper::readRecipe(&recipe);

  // Generate settings file according to recipe
  QTemporaryFile settingsFile;
  QVERIFY(settingsFile.open() == true);

  QSettings settings(settingsFile.fileName(), QSettings::IniFormat);

  UpgradeTestHelper::fillSettings(&settings, testCases);

  // Generate defaults file according to recipe
  QTemporaryFile defaultSettingsFile;
  QVERIFY(defaultSettingsFile.open() == true);

  QSettings defaultSettings(defaultSettingsFile.fileName(), QSettings::IniFormat);

  UpgradeTestHelper::fillDefaultSettings(&defaultSettings, testCases);

  // Parse settings -- do upgrade
#if 0
  settingsFile.seek(0);
  defaultSettingsFile.seek(0);
  qDebug() << "SETTINGS {{{\n" << settingsFile.readAll() << "\n}}}";
  qDebug() << "DEFAULT SETTINGS {{{\n" << defaultSettingsFile.readAll() << "\n}}}";
#endif

  SsuSettings ssuSettings(settingsFile.fileName(), QSettings::IniFormat,
      defaultSettingsFile.fileName());

#if 0
  settingsFile.seek(0);
  qDebug() << "SETTINGS UPGRADED {{{\n" << settingsFile.readAll() << "\n}}}";
#endif

  // Record data for verification phase
  QTest::addColumn<bool>("keyIsSet");
  QTest::addColumn<bool>("keyShouldBeSet");
  QTest::addColumn<QString>("actualValue");
  QTest::addColumn<QString>("expectedValue");

  foreach (const UpgradeTestHelper::TestCase &testCase, testCases){
    foreach (const QString &group, UpgradeTestHelper::groups()){
      const QString key = group.isEmpty() ? testCase.key() : group + '/' + testCase.key();
      QTest::newRow(qPrintable(QString("%1%2:%3:%4")
          .arg(group.isEmpty() ? "" : group + "/")
          .arg(testCase.history())
          .arg(testCase.current())
          .arg(testCase.expected())))
        << ssuSettings.contains(key)
        << testCase.keyShouldBeSet()
        << ssuSettings.value(key).toString()
        << testCase.expected();
    }
  }
}

void SettingsTest::testUpgrade(){
  QFETCH(bool, keyIsSet);
  QFETCH(bool, keyShouldBeSet);
  QFETCH(QString, actualValue);
  QFETCH(QString, expectedValue);

  QCOMPARE(keyIsSet, keyShouldBeSet);
  if (keyIsSet){
    QCOMPARE(actualValue, expectedValue);
  }
}
