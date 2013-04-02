/**
 * @file main.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2012
 */

#include <QtTest/QtTest>

#include "ssuurlresolvertest.h"

int main(int argc, char **argv){
  SsuUrlResolverTest ssuUrlResolverTest;

  if (QTest::qExec(&ssuUrlResolverTest, argc, argv))
    return 1;

  return 0;
}
