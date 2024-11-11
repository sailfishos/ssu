/**
 * @file ssud.h
 * @copyright 2013-2019 Jolla Ltd.
 * @copyright 2019 Open Mobile Platform LLC.
 * @copyright LGPLv2+
 * @date 2013 - 2019
 */

/*
 *  This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _SSUD_H
#define _SSUD_H

#include <QObject>
#include <QString>
#include <QTimer>

#include "libssu/ssu.h"
#include "ssud_dbus.h"

class Ssud: public QObject
{
    Q_OBJECT

public:
    Ssud(QObject *parent = NULL);
    virtual ~Ssud();

public slots:
    /* device info */
    QString deviceModel();
    QString deviceFamily();
    QString deviceUid();
    QString deviceVariant();
    QString displayName(int type);
    /* credential management */
    bool isRegistered();
    void registerDevice(const QString &username, const QString &password);
    void unregisterDevice();
    QString domain();
    /* repository management */
    QString brand();
    int deviceMode();
    void setDeviceMode(int mode);
    void setDeviceMode(int mode, int editMode);
    QString flavour();
    void setFlavour(const QString &release);
    QString release(bool rnd);
    void setRelease(const QString &release, bool rnd);
    void modifyRepo(int action, const QString &repo);
    void addRepo(const QString &repo, const QString &url);
    void updateRepos();
    QList<SsuRepo> listRepos(bool rnd);
    QStringList listDomains();
    void setDomainConfig(const QString &domain, QVariantMap config);
    QVariantMap getDomainConfig(const QString &domain);

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
