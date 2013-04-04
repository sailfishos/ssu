/**
 * @file ssuurlresolver.h
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#ifndef _SsuUrlResolver_H
#define _SsuUrlResolver_H

#include <QObject>
#include <QSettings>
#include <QDebug>
#include <QEventLoop>
#include <QFile>

#include <iostream>
#include <zypp/PluginFrame.h>

#include "libssu/ssu.h"

// quick hack for waiting for a signal
class SignalWait: public QObject {
    Q_OBJECT
  public:
    SignalWait(){
      needRunning=1;
    }

  public slots:
    void sleep(){
      if (needRunning==1)
        loop.exec();
    }

    virtual void finished(){
      needRunning=0;
      loop.exit();
    }

  private:
    QEventLoop loop;
    int needRunning;
};

using namespace zypp;

class SsuUrlResolver: public QObject {
    Q_OBJECT

  public:
    SsuUrlResolver();

  private:
    Ssu ssu;
    void printJournal(int priority, QString message);
    bool writeCredentials(QString filePath, QString credentialsScope);

  public slots:
    void run();

  signals:
    void done();

};

#endif
