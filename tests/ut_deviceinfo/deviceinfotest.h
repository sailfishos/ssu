/**
 * @file deviceinfotest.h
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#ifndef _DEVICEINFOTEST_H
#define _DEVICEINFOTEST_H

#include <QObject>

class DeviceInfoTest: public QObject {
    Q_OBJECT

  private slots:
    void testAdaptationVariables();
    void testDeviceUid();
    void testVariableSection();
    void testValue();
};

#endif
