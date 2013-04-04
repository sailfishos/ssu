/**
 * @file ssukickstarter.h
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#ifndef _SSUKICKSTARTER_H
#define _SSUKICKSTARTER_H

#include <QObject>
#include <QSettings>
#include <QHash>

#include "libssu/ssudeviceinfo.h"
#include "libssu/ssu.h"

/*
class Q_CORE_EXPORT SsuKickstarter: public QObject {
    Q_OBJECT
*/
class SsuKickstarter {
  public:
    SsuKickstarter();
    void setRepoParameters(QHash<QString, QString> parameters);
    void write(QString kickstart="");

  private:
    QHash<QString, QString> repoOverride;
    Ssu ssu;
    bool rndMode;
    QString deviceModel;
    QStringList commands();
    QStringList packages();
    QStringList partitions();
    QStringList repos();
    QStringList scriptletSection(QString name, bool chroot=true);
};

#endif
