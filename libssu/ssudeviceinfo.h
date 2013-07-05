/**
 * @file ssudeviceinfo.h
 * @copyright 2013 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#ifndef _SSUDEVICEINFO_H
#define _SSUDEVICEINFO_H

#include <QObject>

#include "ssusettings.h"
#include "ssurepomanager.h"

class SsuDeviceInfo: public QObject {
    Q_OBJECT

  public:
    /**
     * Initialize with device to override autodetection
     */
    SsuDeviceInfo(QString model="");
    /**
     * Return the list of adaptations used for the set model
     */
    QStringList adaptationRepos();
    /**
     * Resolve adaptation-specific variables for adaptationName, and store them in storageHash
     * Returns "adaptation" if a valid adaptation was found, adaptationName otherwise
     */
    QString adaptationVariables(const QString &adaptationName, QHash<QString, QString> *storageHash);
    /**
     * Check if a given model is available in the deviceinfo database, either directly,
     * or as variant. If no model is provided as argument the autodetected or previously
     * set model is used.
     */
    bool contains(const QString &model="");
    /**
     * Try to find the device family for the system this is running on. This function
     * temporarily changes the detected model, and therefore should not be used in a
     * multithreaded environment, unless you like funny results.
     */
    Q_INVOKABLE QString deviceFamily();
    /**
     * Try to find the device variant for the system this is running on.
     * If the device is not a variant it will return an empty string. If
     * fallback is set to true it return the device model in this case.
     */
    Q_INVOKABLE QString deviceVariant(bool fallback=false);
    /**
     * Try to find out ond what kind of system this is running
     */
    Q_INVOKABLE QString deviceModel();
    /**
     * Calculate the device ID used in SSU requests
     * @return QSystemDeviceInfo::imei(), if available, or QSystemDeviceInfo::uniqueDeviceID()
     */
    Q_INVOKABLE QString deviceUid();
    /**
     * Return the list of repositories explicitely disabled for this device
     * This does not include repositories only disabled in the user configuration.
     */
    QStringList disabledRepos();
    /**
     * Return the complete list of repositories configured for a device.
     * Depending on the filter options, all repostories (user and board),
     * only board-specific, or only user-specific are returned.
     * Disabled repositories are excluded depending on filter settings.
     */
    QStringList repos(bool rnd=false, int filter=SsuRepoManager::NoFilter);
    /**
     * Override device model autodetection
     */
    Q_INVOKABLE void setDeviceModel(QString model="");
    /**
     * Return a variable from the given variable section. 'var'- is automatically
     * prepended to the section name if not specified already. Recursive search
     * through several variable sections (specified in the section) is supported,
     * returned will be the first occurence of the variable.
     */
    QVariant variable(QString section, const QString &key);
    /**
     * Return the requested variable section. 'var-' is automatically
     * prepended to the section name if not specified already.
     */
    void variableSection(QString section, QHash<QString, QString> *storageHash);
    /**
     * Return a value from an adaptation section. Returns an empty string
     * or a given default value if key does not exist.
     */
    QVariant value(const QString &key, const QVariant &value=QVariant());


  private:
    SsuSettings *boardMappings;
    QString cachedFamily, cachedModel, cachedVariant;

    void clearCache();
};
#endif
