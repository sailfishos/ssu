/**
 * @file ssusettings.h
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#ifndef _SSUSETTINGS_H
#define _SSUSETTINGS_H

#include <QSettings>

class SsuSettings: public QSettings {
    Q_OBJECT

    friend class SettingsTest;

  public:
    SsuSettings();
    SsuSettings(const QString &fileName, Format format, QObject *parent=0);
    /**
     * Initialize the settings object with a defaults settings file, resulting in
     * update to the configuration file if needed
     */
    SsuSettings(const QString &fileName, Format format, const QString &defaultFileName, QObject *parent=0);
    /**
     * Initialize the settings object from a settings.d structure, if needed. Only INI
     * style settings are supported in this mode.
     */
    SsuSettings(const QString &fileName, const QString &settingsDirectory, QObject *parent=0);

  private:
    QString defaultSettingsFile, settingsd;
    void merge(bool keepOld=false);
    static void merge(QSettings *masterSettings, const QStringList &settingsFiles);
    void upgrade();

};

#endif
