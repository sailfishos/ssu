/**
 * @file ssuclitest.h
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#ifndef _SSUCLITEST_H
#define _SSUCLITEST_H

#include <QObject>

class QProcess;

class Sandbox;

class SsuCliTest: public QObject
{
    Q_OBJECT

public:
    SsuCliTest(): m_sandbox(0) {}

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
