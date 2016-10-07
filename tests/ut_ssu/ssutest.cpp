/**
 * @file repomanagertest.cpp
 * @copyright 2015 Jolla Ltd.
 * @author Marko Saukko <marko.saukko@jolla.com>
 * @date 2015
 * @todo Add unit tests for other ssu lib functions.
 */

#include "ssutest.h"

#include <QtTest/QtTest>

#include "libssu/ssucoreconfig_p.h"
#include "libssu/ssurepomanager.h"
#include "libssu/ssu.h"

void SsuTest::initTestCase()
{
}

void SsuTest::testCredentialsScope()
{
    // For store repo store is returned always no matter what
    QCOMPARE(ssu.credentialsScope(QString("store")), QString("store"));
    QCOMPARE(ssu.credentialsScope(QString("store-c-example")), QString("store"));

    ssu.setDomain(QString("default"));
    QCOMPARE(ssu.domain(), QString("default"));

    QCOMPARE(ssu.credentialsScope(QString("store")), QString("store"));
    QCOMPARE(ssu.credentialsScope(QString("apps")), QString("vendor"));
    QCOMPARE(ssu.credentialsScope(QString("vendor")), QString("vendor"));

    QCOMPARE(ssu.credentialsScope(QString("store"), true), QString("store"));
    QCOMPARE(ssu.credentialsScope(QString("adaptation"), true), QString("vendor"));
    QCOMPARE(ssu.credentialsScope(QString("oss"), true), QString("vendor"));
}

void SsuTest::testCredentialsScopeStoreAuthRepos()
{
    ssu.setDomain(QString("example"));
    QCOMPARE(ssu.domain(), QString("example"));

    QCOMPARE(ssu.credentialsScope(QString("store")), QString("store"));
    // store-auth-repos=apps in example domain
    QCOMPARE(ssu.credentialsScope(QString("apps")), QString("store"));
    QCOMPARE(ssu.credentialsScope(QString("vendor")), QString("vendor"));

    QCOMPARE(ssu.credentialsScope(QString("store"), true), QString("store"));
    QCOMPARE(ssu.credentialsScope(QString("adaptation"), true), QString("vendor"));
    QCOMPARE(ssu.credentialsScope(QString("oss"), true), QString("vendor"));
}

void SsuTest::testCredentialsScopeSecureDomainAuth()
{
    //  [secure-domain-auth]
    //  packages.example2.com=example2

    ssu.setDomain("example2");
    QCOMPARE(ssu.domain(), QString("example2"));

    QCOMPARE(ssu.credentialsScope(QString("store"), false), QString("store"));
    QCOMPARE(ssu.credentialsScope(QString("apps"), false), QString("example2"));
    QCOMPARE(ssu.credentialsScope(QString("vendor"), false), QString("example2"));

    // adaptation uses packages.example2.com
    QCOMPARE(ssu.credentialsScope(QString("adaptation"), true), QString("example2"));
    // oss uses dump.example2.com, thus vendor
    QCOMPARE(ssu.credentialsScope(QString("oss"), true), QString("vendor"));
}

