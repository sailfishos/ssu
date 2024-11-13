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
#include  <zypp/APIConfig.h>
#if LIBZYPP_VERSION >= 173402
#include <zypp-core/rpc/PluginFrame.h>
#else
#include <zypp/PluginFrame.h>
#endif

#include "libssu/ssu.h"

// quick hack for waiting for a signal
class SignalWait: public QObject
{
    Q_OBJECT
public:
    SignalWait()
    {
        needRunning = 1;
    }

public slots:
    void sleep()
    {
        if (needRunning == 1)
            loop.exec();
    }

    virtual void finished()
    {
        needRunning = 0;
        loop.exit();
    }

private:
    QEventLoop loop;
    int needRunning;
};

using namespace zypp;

class SsuUrlResolver: public QObject
{
    Q_OBJECT

public:
    SsuUrlResolver();

private:
    Ssu ssu;
    void error(const QString &message);
    void ack() const;
    void resolve(zypp::PluginFrame &in);
    bool writeZyppCredentialsIfNeeded(const QString &credentialsScope);

public slots:
    void run();

signals:
    void done();
};

#endif
