/**
 * @file coreconfigtest.h
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#ifndef _CORECONFIGTEST_H
#define _CORECONFIGTEST_H

#include <QObject>

class CoreconfigTest: public QObject {
    Q_OBJECT

  private slots:
    void testCredentialsScope();
    void testCredentials();
    void testCredentialsUrl();
    void testFlavour();
    void testDeviceMode();
    void testDomain();
    void testRegistered();
    void testLastCredentialsUpdate();
    void testRelease();
    void testSslVerify();
};

#endif
