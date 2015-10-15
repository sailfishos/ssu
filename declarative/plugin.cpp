/**
 * @file plugin.cpp
 * @copyright 2015 Jolla Ltd.
 * @author Joona Petrell <joona.petrell@jollamobile.com>
 * @date 2015
 */

#include <QQmlExtensionPlugin>
#include <QQmlEngine>

#include <qqml.h>
#include "declarativessufeaturemodel.h"

class NemoSsuPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.nemomobile.ssu")

public:
    virtual void registerTypes(const char *)
    {
        qmlRegisterType<DeclarativeSsuFeatureModel>("Nemo.Ssu", 1, 0, "FeatureModel");
    }
};

#include "plugin.moc"
