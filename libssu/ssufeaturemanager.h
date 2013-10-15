/**
 * @file ssufeaturemanager.h
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#ifndef _SSUFEATUREMANAGER_H
#define _SSUFEATUREMANAGER_H

#include <QObject>
#include <QHash>
#include <QStringList>

#include "ssusettings.h"

class SsuFeatureManager: public QObject {
    Q_OBJECT

  public:
    SsuFeatureManager(){};
    // add rnd-flag?
    QStringList repos(){};
    QString url(QString repo){ return ""; };

  private:
    SsuSettings *featureSettings;
};

#endif
