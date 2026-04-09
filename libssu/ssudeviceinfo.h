/**
 * @file ssudeviceinfo.h
 * @copyright 2013 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#ifndef _SSUDEVICEINFO_H
#define _SSUDEVICEINFO_H

#include <QObject>

#include "ssu.h"

/**
 * Represents the currently allowed sources for determening the device id.
 * Unset configuration enables all sources to maintain backwards compatibility.
 *
 * The board mapping configuration key is "device-id-source" which defaults to value "any".
 */
struct SsuDeviceIdSource {
    /**
     * Allow using the IMEI code, if any, as basis of device id. Configuration value: "imei"
     */

    bool imei = false;
    /**
     * Allow using the WLAN adapter MAC address, if any, as basis of device id. Configuration value: "wlan-mac"
     */

    bool wlanMac = false;
    /**
     * Allow using the machine-id, as basis of device id. Configuration value: "machine-id"
     *
     * There are several locations where the read is attempted,
     * see the source code of SsuDeviceInfo::deviceUid for details.
     */
    bool machineId = false;
};

inline bool operator==(SsuDeviceIdSource lhs, SsuDeviceIdSource rhs) {
    return lhs.imei == rhs.imei
        && lhs.wlanMac == rhs.wlanMac
        && lhs.machineId == rhs.machineId;
}

class SsuSettings;

class SsuDeviceInfo: public QObject
{
    Q_OBJECT

public:
    /**
     * Initialize with device to override autodetection
     */
    SsuDeviceInfo(const QString &model = QString());

    virtual ~SsuDeviceInfo();

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
     *
     * @attention This function temporarily changes the detected model, and
     * therefore should not be used in a multithreaded environment, unless you
     * like funny results.
     */
    bool contains(const QString &model = QString());

    /**
     * Try to find the device family for the system this is running on.
     */
    Q_INVOKABLE QString deviceFamily();

    /**
     * Try to find the device variant for the system this is running on.
     * If the device is not a variant it will return an empty string. If
     * fallback is set to true it return the device model in this case.
     */
    Q_INVOKABLE QString deviceVariant(bool fallback = false);

    /**
     * Try to find out ond what kind of system this is running
     */
    Q_INVOKABLE QString deviceModel();

    /**
     * Read the board mapping configuration and determine the allowed source, or sources, of device id.
     *
     * The configuration can either allow a single source, or all/any sources at once.
     */
    SsuDeviceIdSource deviceIdSource();

    /**
     * Calculate the device ID used in ssu requests, according to board mapping configuration, if any.
     * Without any configuration, return the first imei from oFono ModemManager API, or WLAN mac address,
     * or device uid fallback code similar to QDeviceInfo::uniqueDeviceID().
     * @return The device id string
     */
    Q_INVOKABLE QString deviceUid();

    /**
     * Return the list of repositories explicitely disabled for this device
     * This does not include repositories only disabled in the user configuration.
     */
    QStringList disabledRepos();

    /**
     * Return a string suitable for display in dialogs, ...
     *
     * See Ssu::DeviceTypes for supported types.
     *
     * If not configured the model name used by ssu will be returned instead
     * for product and type.
     * If no manufacturer is found UNKNOWN is returned.
     * For an invalid type an empty string is returned.
     */
    Q_INVOKABLE QString displayName(int type);

    /**
     * Return the complete list of repositories configured for a device.
     * Depending on the filter options, all repostories (user and board),
     * only board-specific, or only user-specific are returned.
     * Disabled repositories are excluded depending on filter settings.
     */
    QStringList repos(bool rnd = false, int filter = Ssu::NoFilter);

    /**
     * Override device model autodetection
     */
    Q_INVOKABLE void setDeviceModel(const QString &model = QString());

    /**
     * Return a variable from the given variable section. 'var'- is automatically
     * prepended to the section name if not specified already. Recursive search
     * through several variable sections (specified in the section) is supported,
     * returned will be the first occurence of the variable.
     */
    QVariant variable(const QString &section, const QString &key);

    /**
     * Return the requested variable section. 'var-' is automatically
     * prepended to the section name if not specified already.
     */
    void variableSection(const QString &section, QHash<QString, QString> *storageHash);

    /**
     * Return a value from an adaptation section. Returns an empty string
     * or a given default value if key does not exist.
     *
     * If a device is marked as variant, and the requested key exists in a
     * dedicated device section the key from the device section will get
     * returned, otherwise the one from the variant.
     */
    QVariant value(const QString &key, const QVariant &value = QVariant());

private:
    SsuSettings *boardMappings;
    QString cachedFamily, cachedModel, cachedVariant;

    void clearCache();

    /**
     * Return a map of key, value pairs parsed from /etc/hw-release
     */
    QMap<QString, QString> hwRelease();
};
#endif
