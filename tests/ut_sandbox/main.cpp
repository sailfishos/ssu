/**
 * @file main.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2012
 */

#include <QtTest/QtTest>

#include "sandboxtest.h"

int main(int argc, char **argv){
  SandboxTest sandboxTest;

  if (QTest::qExec(&sandboxTest, argc, argv))
    return 1;

  return 0;
}
