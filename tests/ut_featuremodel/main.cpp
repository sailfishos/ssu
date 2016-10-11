/**
 * @file main.cpp
 * @copyright 2015 Jolla Ltd.
 * @author Joona Petrell <joona.petrell@jolla.com>
 * @date 2015
 */

#include <QtCore/QCoreApplication>
#include <QtTest/QtTest>

#include "featuremodeltest.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    FeatureModelTest featureModelTest;

    if (QTest::qExec(&featureModelTest, argc, argv))
        return 1;

    return 0;
}
