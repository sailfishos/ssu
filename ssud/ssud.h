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
#include <QTimer>

#include "libssu/ssu.h"

class Ssud: public QObject {
    Q_OBJECT

  public:
    Ssud(QObject *parent=NULL);
    virtual ~Ssud();

  public slots:
    /* device info */
    QString deviceModel();
    QString deviceFamily();
    QString deviceUid();
    QString deviceVariant();
    /* credential management */
    bool isRegistered();
    void registerDevice(const QString &username, const QString &password);
    void unregisterDevice();
    /* repository management */
    int deviceMode();
    void setDeviceMode(int mode);
    QString flavour();
    void setFlavour(const QString &release);
    QString release(bool rnd);
    void setRelease(const QString &release, bool rnd);
    void modifyRepo(int action, const QString &repo);
    void addRepo(const QString &repo, const QString &url);
    void updateRepos();

    bool error();
    QString lastError();

    void quit();


  signals:
    void done();
    void credentialsChanged();
    void registrationStatusChanged();

  private:
    Ssu ssu;
    static const char *SERVICE_NAME;
    static const char *OBJECT_PATH;
    QTimer autoclose;

    enum Actions {
      Remove  = 0,
      Add     = 1,
      Disable = 2,
      Enable  = 3,
    };
};

#endif
