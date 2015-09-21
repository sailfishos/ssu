/**
 * @file repomanagertest.h
 * @copyright 2015 Jolla Ltd.
 * @author Marko Saukko <marko.saukko@jolla.com>
 * @date 2015
 */

#ifndef _SSUTEST_H
#define _SSUTEST_H

#include <QObject>
#include <QStringList>

#include "libssu/ssu.h"

class SsuTest: public QObject {
    Q_OBJECT

  private slots:
    void initTestCase();
    void testCredentialsScope();
    void testCredentialsScopeStoreAuthRepos();
    void testCredentialsScopeSecureDomainAuth(); 
  private:
    Ssu ssu;
};

#endif
