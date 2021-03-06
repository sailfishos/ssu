/**
 * @file ssuurlresolvertest.h
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#ifndef _SSUURLRESOLVERTEST_H
#define _SSUURLRESOLVERTEST_H

#include <QObject>

class Sandbox;

class SsuUrlResolverTest: public QObject
{
    Q_OBJECT

public:
    SsuUrlResolverTest(): m_sandbox(0) {}

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_data();
    void test();

private:
    Sandbox *m_sandbox;
};

#endif
