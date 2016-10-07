/**
 * @file main.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#include <QtTest/QtTest>

#include "variablestest.h"

int main(int argc, char **argv)
{
    VariablesTest variablesTest;

    if (QTest::qExec(&variablesTest, argc, argv))
        return 1;

    return 0;
}
