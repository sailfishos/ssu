/**
 * @file settingstest.h
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#ifndef _SETTINGSTEST_H
#define _SETTINGSTEST_H

#include <QObject>

class SettingsTest: public QObject {
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testMerge_data();
    void testMerge();
    void testUpgrade_data();
    void testUpgrade();

  private:
};

#endif
