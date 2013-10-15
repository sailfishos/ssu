/**
 * @file rndssuclitest.h
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#ifndef _RNDSSUCLITEST_H
#define _RNDSSUCLITEST_H

#include <QObject>

class QProcess;

class Sandbox;

class RndSsuCliTest: public QObject {
    Q_OBJECT

  public:
    RndSsuCliTest(): m_sandbox(0) {}

  private slots:
    void init();
    void cleanup();

    void testSubcommandFlavour();
    void testSubcommandRelease();
    void testSubcommandMode();

  private:
    Sandbox *m_sandbox;
    QProcess *m_bus;
    QProcess *m_ssud;
};

#endif
