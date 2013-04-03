/**
 * @file rndssuclitest.h
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#ifndef _RNDSSUCLITEST_H
#define _RNDSSUCLITEST_H

#include <QObject>

class RndSsuCliTest: public QObject {
    Q_OBJECT

    class Process;

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testSubcommandFlavour();
    void testSubcommandRelease();
    void testSubcommandMode();

  private:
    QString tempDir;
};

#endif
