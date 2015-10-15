/**
 * @file featuremodeltest.h
 * @copyright 2015 Jolla Ltd.
 * @author Joona Petrell <joona.petrell@jolla.com>
 * @date 2015
 */

#ifndef _FEATUREMODELTEST_H
#define _FEATUREMODELTEST_H

#include <QObject>

class FeatureModelTest: public QObject {
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testFeatures();
};

#endif
