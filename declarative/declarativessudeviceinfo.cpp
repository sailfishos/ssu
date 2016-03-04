/**
 * @file declarativessufeaturemodel.cpp
 * @copyright 2016 Jolla Ltd.
 * @author Martin Jones <martin.jones@jolla.com>
 * @date 2016
 */

#include "declarativessudeviceinfo.h"

DeclarativeSsuDeviceInfo::DeclarativeSsuDeviceInfo()
{
}

QString DeclarativeSsuDeviceInfo::deviceFamily()
{
    return info.deviceFamily();
}

QString DeclarativeSsuDeviceInfo::deviceVariant(bool fallback)
{
    return info.deviceVariant(fallback);
}

QString DeclarativeSsuDeviceInfo::deviceModel()
{
    return info.deviceModel();
}

QString DeclarativeSsuDeviceInfo::displayName(const int type)
{
    return info.displayName(type);
}

