/**
 * @file main.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2012
 */

#include <QtCore/QCoreApplication>
#include <QtTest/QtTest>

#include "settingstest.h"
#include "upgradetesthelper.h"

int main(int argc, char **argv){
  QCoreApplication app(argc, argv);

  if (app.arguments().contains("-generate-upgrade-test-recipe")){
    QTextStream out(stdout);
    return UpgradeTestHelper::generateSnapshotRecipe(&out) ? 0 : 1;
  }

  SettingsTest settingsTest;

  if (QTest::qExec(&settingsTest, argc, argv))
    return 1;

  return 0;
}
