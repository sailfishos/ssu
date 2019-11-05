/**
 * @file ssusettings_p.h
 * @copyright 2013 - 2019 Jolla Ltd.
 * @copyright 2019 Open Mobile Platform LLC.
 * @copyright LGPLv2+
 * @date 2013 - 2019
 */

/*
 *  This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _SSUSETTINGS_P_H
#define _SSUSETTINGS_P_H

#include <QSettings>

class SsuSettings: public QSettings
{
    Q_OBJECT

    friend class SettingsTest;

public:
    SsuSettings();
    SsuSettings(const QString &fileName, Format format, QObject *parent = 0);
    /**
     * Initialize the settings object with a defaults settings file, resulting in
     * update to the configuration file if needed
     */
    SsuSettings(const QString &fileName, Format format, const QString &defaultFileName, QObject *parent = 0);
    /**
     * Initialize the settings object from a settings.d structure, if needed. Only INI
     * style settings are supported in this mode.
     */
    SsuSettings(const QString &fileName, const QString &settingsDirectory, QObject *parent = 0);

private:
    QString defaultSettingsFile, settingsd;
    void merge(bool keepOld = true);
    static void merge(QSettings *masterSettings, const QStringList &settingsFiles);
    void upgrade();

};

#endif
