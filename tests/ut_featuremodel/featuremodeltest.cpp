/**
 * @file featuremodeltest.cpp
 * @copyright 2015 Jolla Ltd.
 * @author Joona Petrell <joona.petrell@jolla.com>
 * @date 2015
 */

#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QObject>

#include "libssu/ssufeaturemodel.h"

class FeatureModelTest: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testFeatures();
};

void FeatureModelTest::initTestCase()
{
}

void FeatureModelTest::cleanupTestCase()
{
}

void FeatureModelTest::testFeatures()
{
    SsuFeatureModel featureModel(0, ":/testdata");

    // Features with no name are skipped (feature-noname.ini)
    QCOMPARE(featureModel.count(), 3);

    // First feature is "Feature A" in version 0.1

    // Explicit getter
    const QHash<QByteArray, QString> featureA = featureModel.featureAt(0);
    QCOMPARE(featureA.value("name"), QString("Feature A"));
    QCOMPARE(featureA.value("version"), QString("0.1"));

    // Through QAbstractListModel API
    QCOMPARE(featureModel.data(featureModel.index(0), SsuFeatureModel::Name).toString(), QString("Feature A"));
    QCOMPARE(featureModel.data(featureModel.index(0), SsuFeatureModel::Version).toString(), QString("0.1"));

    // Second feature is "Feature B" in version 0.2
    const QHash<QByteArray, QString> featureB = featureModel.featureAt(1);
    QCOMPARE(featureB.value("name"), QString("Feature B"));
    QCOMPARE(featureB.value("version"), QString("0.2"));

    // Third feature lacks version number
    const QHash<QByteArray, QString> featureWithoutVersion = featureModel.featureAt(2);
    QCOMPARE(featureWithoutVersion.value("name"), QString("Feature without version"));
    QCOMPARE(featureWithoutVersion.value("version"), QString(""));
}

QTEST_GUILESS_MAIN(FeatureModelTest)
#include "featuremodeltest.moc"
