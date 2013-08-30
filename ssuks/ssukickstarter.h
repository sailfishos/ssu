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

class SsuKickstarter {
  public:
    SsuKickstarter();
    void setRepoParameters(QHash<QString, QString> parameters);
    bool write(QString kickstart="");

    enum ScriptletFlags {
      /// Chroot is not useful, but helps in making the code more readable
      Chroot         = 0,
      NoChroot       = 0x1,
      DeviceSpecific = 0x2,
    };

  private:
    QHash<QString, QString> repoOverride;
    Ssu ssu;
    bool rndMode;
    QString deviceModel;
    QStringList commands();
    /// read a command section from file system
    QStringList commandSection(const QString &section, const QString &description="");
    QStringList packagesSection(QString name);
    QString replaceSpaces(const QString &value);
    QStringList repos();
    QStringList scriptletSection(QString name, int flags=Chroot);
};

#endif
