/**
 * @file ssuurlresolvertest.h
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#ifndef _SSUURLRESOLVERTEST_H
#define _SSUURLRESOLVERTEST_H

#include <QObject>

class SsuUrlResolverTest: public QObject {
    Q_OBJECT

  private slots:
    void initTestCase();
    void test_data();
    void test();

  private:
};

#endif
