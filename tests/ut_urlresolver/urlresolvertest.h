/**
 * @file urlresolvertest.h
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2012
 */

#ifndef _URLRESOLVERTEST_H
#define _URLRESOLVERTEST_H

#include <QObject>
#include <QtTest/QtTest>
#include <QHash>

#include "libssu/ssu.h"

class UrlResolverTest: public QObject {
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void checkFlavour();
    void checkRelease();
    void checkDomain();
    void checkCleanUrl();
    void simpleRepoUrlLookup();
    void checkReleaseRepoUrls();

  private:
    Ssu ssu;
    QHash<QString, QString> rndRepos, releaseRepos;
};

#endif
