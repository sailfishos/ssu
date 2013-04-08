/**
 * @file sandboxtest.h
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#ifndef _SANDBOXTEST_H
#define _SANDBOXTEST_H

#include <QObject>

class SandboxTest: public QObject {
    Q_OBJECT

  private slots:
    void test();

  private:
    static QString readAll(const QString &fileName);
};

#endif
