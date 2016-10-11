/**
 * @file main.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2012
 */

#include <QtTest/QtTest>

#include "ssuclitest.h"

int main(int argc, char **argv)
{
    SsuCliTest ssuCliTest;

    if (QTest::qExec(&ssuCliTest, argc, argv))
        return 1;

    return 0;
}
