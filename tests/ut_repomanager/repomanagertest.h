/**
 * @file repomanagertest.h
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#ifndef _REPOMANAGERTEST_H
#define _REPOMANAGERTEST_H

#include <QObject>
#include <QStringList>

class RepoManagerTest: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testSettings();
    void testCustomRepos();
    void testRepos();

private:
    QStringList rndRepos, releaseRepos;
};

#endif
