/**
 * @file main.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2012
 */

#include <QtTest/QtTest>

#include "settingstest.h"

int main(int argc, char **argv){
  SettingsTest settingsTest;

  if (QTest::qExec(&settingsTest, argc, argv))
    return 1;

  return 0;
}
