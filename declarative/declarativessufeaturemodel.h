/**
 * @file declarativessufeaturemodel.h
 * @copyright 2015 Jolla Ltd.
 * @author Joona Petrell <joona.petrell@jolla.com>
 * @date 2015
 */

#ifndef _DECLARATIVESSUFEATUREMODEL_H
#define _DECLARATIVESSUFEATUREMODEL_H

#include "../libssu/ssufeaturemodel.h"
#include <QJSValue>

class DeclarativeSsuFeatureModel: public SsuFeatureModel
{
    Q_OBJECT
public:
    Q_INVOKABLE QJSValue get(int index) const;
};

#endif


