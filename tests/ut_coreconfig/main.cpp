/**
 * @file main.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2012
 */

#include <QtTest/QtTest>

#include "libssu/sandbox_p.h"
#include "coreconfigtest.h"

int main(int argc, char **argv){
  Sandbox sandbox(QString("%1/configroot").arg(TESTS_DATA_PATH),
      Sandbox::UseAsSkeleton, Sandbox::ThisProcess);
  if (!sandbox.activate()){
    qFatal("Failed to activate sandbox");
  }

  CoreconfigTest coreconfigTest;

  if (QTest::qExec(&coreconfigTest, argc, argv))
    return 1;

  return 0;
}
