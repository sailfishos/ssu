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
    bool write(QString kickstart="");

  private:
    QHash<QString, QString> repoOverride;
    Ssu ssu;
    bool rndMode;
    QString deviceModel;
    QStringList commands();
    /// read a command section from file system
    QStringList commandSection(const QString &section, const QString &description="");
    QStringList packages();
    QString replaceSpaces(const QString &value);
    QStringList repos();
    QStringList scriptletSection(QString name, bool chroot=true);
};

#endif
