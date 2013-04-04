/**
 * @file variablestest.h
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#ifndef _VARIABLESTEST_H
#define _VARIABLESTEST_H

#include <QObject>
#include <QtTest/QtTest>
#include <QHash>

#include "libssu/ssu.h"
#include "libssu/ssuvariables.h"

class VariablesTest: public QObject {
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void checkResolveString();


  private:
    Ssu ssu;
    SsuVariables var;
    QHash <QString, QString> variables, urls;
};

#endif
