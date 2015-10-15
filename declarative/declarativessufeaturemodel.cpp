/**
 * @file declarativessufeaturemodel.cpp
 * @copyright 2015 Jolla Ltd.
 * @author Joona Petrell <joona.petrell@jolla.com>
 * @date 2015
 */

#include "declarativessufeaturemodel.h"
#include <QHash>
#include <qqml.h>
#include <QQmlEngine>
#include <qqmlinfo.h>

QJSValue DeclarativeSsuFeatureModel::get(int index) const
{
    if (index < 0 || index >= count()) {
        qmlInfo(this) << "Index" << index << "out of bounds";
        return QJSValue();
    }
    const QHash<QByteArray, QString> feature = featureAt(index);

    QJSEngine *const engine = qmlEngine(this);
    QJSValue value = engine->newObject();

    QHash<QByteArray, QString>::const_iterator i = feature.constBegin();
    while (i != feature.constEnd()) {
        value.setProperty(QString::fromLatin1(i.key()), i.value());
        ++i;
    }

    return value;
}
