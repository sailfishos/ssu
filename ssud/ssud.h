/**
 * @file ssud.h
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#ifndef _SSUD_H
#define _SSUD_H

#include <QObject>
#include <QString>

#include "libssu/ssu.h"

class Ssud: public QObject {
    Q_OBJECT

  public:
    Ssud(QObject *parent=NULL);
    virtual ~Ssud();

  public slots:
    QString deviceModel();
    QString deviceUid();
    bool error();
    QString lastError();
    bool isRegistered();
    void quit();
    void registerDevice(const QString &username, const QString &password);
    void unregisterDevice();

  signals:
    void done();
    void credentialsChanged();
    void registrationStatusChanged();

  private:
    Ssu ssu;
    static const char *SERVICE_NAME;
    static const char *OBJECT_PATH;
};

#endif
