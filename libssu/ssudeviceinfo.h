/**
 * @file ssudeviceinfo.h
 * @copyright 2013 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#ifndef _SSUDEVICEINFO_H
#define _SSUDEVICEINFO_H

#include <QObject>
#include <QSettings>

class SsuDeviceInfo: public QObject {
    Q_OBJECT

  public:
    SsuDeviceInfo();
    /**
     * Try to find the device family for the system this is running on
     */
    Q_INVOKABLE QString deviceFamily();
    /**
     * Try to find the device variant for the system this is running on.
     */
    Q_INVOKABLE QString deviceVariant();
    /**
     * Try to find out ond what kind of system this is running
     */
    Q_INVOKABLE QString deviceModel();
    /**
     * Calculate the device ID used in SSU requests
     * @return QSystemDeviceInfo::imei(), if available, or QSystemDeviceInfo::uniqueDeviceID()
     */
    Q_INVOKABLE QString deviceUid();

    bool getValue(const QString& key, QString& value);

  private:
    QSettings *boardMappings;
    QString cachedFamily, cachedModel, cachedVariant;
};
#endif
