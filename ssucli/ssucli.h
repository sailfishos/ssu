/**
 * @file ssucli.h
 * @copyright 2012 - 2019 Jolla Ltd.
 * @copyright 2019 Open Mobile Platform LLC.
 * @copyright LGPLv2+
 * @date 2012 - 2019
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

#ifndef _SsuCli_H
#define _SsuCli_H

#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QDebug>

#include "libssu/ssu.h"
#include "libssu/ssuproxy.h"

class SsuCli: public QObject
{
    Q_OBJECT

public:
    SsuCli();
    ~SsuCli();

public slots:
    void run();

private:
    Ssu ssu;
    SsuProxy *ssuProxy = nullptr;
    QSettings settings;
    int state;

    SsuProxy *getSsuProxy();
    void usage(const QString &message = QString());
    void uidWarning();
    void optBrand(QStringList opt);
    void optDomain(QStringList opt);
    void optFlavour(QStringList opt);
    QString getModeString(int mode);
    void optMode(QStringList opt);
    void optModel(QStringList opt);
    void optRegister(QStringList opt);
    void optRelease(QStringList opt);
    void optRepos(QStringList opt);
    void optSet(QStringList opt);
    void optStatus(QStringList opt);
    void optUpdateCredentials(QStringList opt);
    void optUpdateRepos(QStringList opt);

    enum Actions {
        Remove,
        Add,
        Disable,
        Enable
    };

    void optModifyRepo(enum Actions action, QStringList opt);

    void optAddRepo(QStringList opt)
    {
        optModifyRepo(Add, opt);
    }
    void optRemoveRepo(QStringList opt)
    {
        optModifyRepo(Remove, opt);
    }
    void optEnableRepo(QStringList opt)
    {
        optModifyRepo(Enable, opt);
    }
    void optDisableRepo(QStringList opt)
    {
        optModifyRepo(Disable, opt);
    }

    enum State {
        Idle,
        Busy,
        UserError
    };

    QString fallingBackToDirectUse(const QDBusError &error) const;

private slots:
    void handleResponse();
    void handleDBusResponse();

signals:
    void done();

};

#endif
