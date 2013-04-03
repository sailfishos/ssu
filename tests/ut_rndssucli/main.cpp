/**
 * @file main.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2012
 */

#include <QtTest/QtTest>

#include "rndssuclitest.h"

int main(int argc, char **argv){
  RndSsuCliTest rndSsuCliTest;

  if (QTest::qExec(&rndSsuCliTest, argc, argv))
    return 1;

  return 0;
}
