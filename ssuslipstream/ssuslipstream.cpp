/**
 * ssu: Seamless Software Update
 * Copyright (C) 2015 Jolla Ltd.
 * Contact: Thomas Perl <thomas.perl@jolla.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **/

#include "ssuslipstream.h"

#include <QCoreApplication>
#include <QTimer>
#include <QUrl>

#include "libssu/ssu.h"
#include "libssu/ssudeviceinfo.h"
#include "libssu/ssucoreconfig.h"


SsuSlipstream::SsuSlipstream()
    : QObject()
{
}

void SsuSlipstream::run()
{
    QTextStream err(stderr, QIODevice::WriteOnly);

    QString partition = QString::fromUtf8(qgetenv("SSU_SLIPSTREAM_PARTITION"));

    if (partition.isEmpty()) {
        err << "Nothing to do\n";
        QCoreApplication::exit(1);
        return;
    }

    Ssu ssu;
    // XXX: Do we need ssu.isRegistered() + updateCredentials() here?
    QPair<QString, QString> credentials = ssu.credentials("store");
    QString release = ssu.release(false);

    QString release_override = QString::fromUtf8(qgetenv("SSU_SLIPSTREAM_RELEASE"));
    if (!release_override.isEmpty()) {
        err << QString("Forcing release to: %1\n").arg(release_override);
        release = release_override;
    }

    SsuCoreConfig *settings = SsuCoreConfig::instance();
    const QString KEY("slipstream-url");
    QString ssuCredentialsUrl = settings->value(KEY, ssu.repoUrl(KEY)).toString();

    if (ssuCredentialsUrl.isEmpty()) {
        err << "URL for slipstream update not set (config key 'slipstream-url')\n";
        QCoreApplication::exit(1);
        return;
    }

    QUrl url(ssuCredentialsUrl);
    url.setUserName(credentials.first);
    url.setPassword(credentials.second);

    QTextStream out(stdout, QIODevice::WriteOnly);
    out << url.toString() << '/' << release << '/' << partition << '\n';

    QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
}
