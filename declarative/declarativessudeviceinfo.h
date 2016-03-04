/**
 * @file declarativessufeaturemodel.h
 * @copyright 2016 Jolla Ltd.
 * @author Martin Jones <martin.jones@jolla.com>
 * @date 2016
 */

#ifndef _DECLARATIVESSUDEVICEINFO_H
#define _DECLARATIVESSUDEVICEINFO_H

#include "../libssu/ssudeviceinfo.h"

class DeclarativeSsuDeviceInfo : public QObject
{
    Q_OBJECT
    Q_ENUMS(DisplayType)
public:
    /**
     * A list of types ssu provides shiny values suitable for displaying
     */
    enum DisplayType {
      DeviceManufacturer = Ssu::DeviceManufacturer,  ///< Manufacturer, like ACME Corp. Board mappings key "deviceManufacturer"
      DeviceModel = Ssu::DeviceModel,                ///< Marketed device name, like Pogoblaster 3000. Board mappings key "prettyModel"
      DeviceDesignation = Ssu::DeviceDesignation,    ///< Type designation, like NCC-1701. Beard mappings key "deviceDesignation"
    };

    DeclarativeSsuDeviceInfo();

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
     * Return a string suitable for display in dialogs, ...
     *
     * See DeviceTypes for supported types.
     *
     * If not configured the model name used by ssu will be returned instead
     * for product and type.
     * If no manufacturer is found UNKNOWN is returned.
     * For an invalid type an empty string is returned.
     */
    Q_INVOKABLE QString displayName(const int type);

private:
    SsuDeviceInfo info;
};

#endif


