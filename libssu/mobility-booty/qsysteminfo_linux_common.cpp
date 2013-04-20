/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsysteminfo_linux_common_p.h"

#include <QTimer>
#include <QFile>
#include <QDir>
#include <QCryptographicHash>
#include <QVariantMap>
#include <QTextStream>
#include <QDebug>

#include <QDBusReply>
#include <QDBusConnectionInterface>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "qofonoservice_linux_p.h"
#include "qsysteminfo_dbus_p.h"

//QTM_BEGIN_NAMESPACE

QSystemDeviceInfoLinuxCommonPrivate::QSystemDeviceInfoLinuxCommonPrivate(QObject *parent)
    : QObject(parent)
{

}

QSystemDeviceInfoLinuxCommonPrivate::~QSystemDeviceInfoLinuxCommonPrivate()
{
}

QString QSystemDeviceInfoLinuxCommonPrivate::imei()
{
#if !defined(QT_NO_CONNMAN)
    if (ofonoAvailable()) {
        QOfonoManagerInterface ofonoManager;
        QString modem = ofonoManager.currentModem().path();
        if (!modem.isEmpty()) {
            QOfonoModemInterface modemIface(modem,this);
            QString imei = modemIface.getSerial();
            if (!imei.isEmpty())
                return imei;
        }
    }
#endif // QT_NO_CONNMAN

    return QString();
}

QString QSystemDeviceInfoLinuxCommonPrivate::manufacturer()
{
    QFile vendorId("/sys/devices/virtual/dmi/id/board_vendor");
    if (vendorId.open(QIODevice::ReadOnly)) {
        QTextStream cpuinfo(&vendorId);
        return cpuinfo.readLine().trimmed();
    } else {
        QFile file("/proc/cpuinfo");
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Could not open /proc/cpuinfo";
        } else {
            QTextStream cpuinfo(&file);
            QString line = cpuinfo.readLine();
            while (!line.isNull()) {
                line = cpuinfo.readLine();
                if (line.contains("vendor_id"))
                    return line.split(": ").at(1).trimmed();
            }
        }
    }

    return QString();
}

QByteArray QSystemDeviceInfoLinuxCommonPrivate::uniqueDeviceID()
{
    QCryptographicHash hash(QCryptographicHash::Sha1);

    // Return the DBUS machine ID
    QByteArray dbusId = QDBusConnection::localMachineId();
    hash.addData(dbusId);

    return hash.result().toHex();
}


//#include "moc_qsysteminfo_linux_common_p.cpp"

//QTM_END_NAMESPACE
