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

  public:
    SsuSettings();
    SsuSettings(const QString &fileName, Format format, QObject *parent=0);
    SsuSettings(const QString &fileName, Format format, const QString &defaultFileName, QObject *parent=0);

  private:
    QString defaultSettingsFile, settingsd;
    void merge();
    void upgrade();

};

#endif
