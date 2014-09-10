/**
 * @file ssurepomanager.h
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#ifndef _SSUREPOMANAGER_H
#define _SSUREPOMANAGER_H

#include <QObject>
#include <QHash>

#include "ssu.h"

class SsuDeviceInfo;

class SsuRepoManager: public QObject {
    Q_OBJECT

  public:
    SsuRepoManager();
    /**
     * Add a repository. Note: Repositories ending with -debuginfo receive special
     * treatment. They'll get saved with the full name to make zypper and the user
     * happy, but internally the -debuginfo will be stripped, and the debugSplit
     * parameter set to debug instead.
     *
     * If the device is in UpdateMode this function does nothing.
     */
    int add(QString repo, QString repoUrl="");
    /**
     * Return the path to the CA certificate to be used for the given domain,
     * or default domain, if omitted
     *
     * @retval 0 Success
     * @retval -1 Repository not added because device is in update mode
     * @retval -2 Repository not added because third party repositories are disabled
     */
    static QString caCertificatePath(QString domain="");
    /**
     * Disable a repository
     *
     * @retval 0 Success
     * @retval -1 Request ignored because device is in update mode
     * @retval -2 Request ignored because 3rd party repositories are disabled
     */
    int disable(QString repo);
    /**
     * Enable a repository, given it's not disabled by board configuration
     *
     * @retval 0 Success
     * @retval -1 Request ignored because device is in update mode
     * @retval -2 Request ignored because 3rd party repositories are disabled
     */
    int enable(QString repo);
    /**
     * Remove a repository
     *
     * @retval 0 Success
     * @retval -1 Request ignored because device is in update mode
     * @retval -2 Request ignored because 3rd party repositories are disabled
     */
    int remove(QString repo);
    /**
     * Collect the list of repositories from different submodules
     */
    QStringList repos(bool rnd, int filter=Ssu::NoFilter);
    /**
     * Collect the list of repositories from different submodules.
     * This form takes a reference to a custom device info instance
     * to allow overrides.
     */
    QStringList repos(bool rnd, SsuDeviceInfo &deviceInfo, int filter=Ssu::NoFilter);
    /**
     * Resolve repository specific variables, and store them in storageHash. Does
     * not include adaptation specific variables, see SsuDeviceInfo::adaptationVariables
     *
     * Returns a list of sections in the configuration file that  might contain repository
     * URLs
     */
    QStringList repoVariables(QHash<QString, QString> *storageHash, bool rnd=false);
    /**
     * Update the repository files on disk
     */
    void update();
    /**
     * Resolve a repository url
     * @return the repository URL on success, an empty string on error
     */
    QString url(QString repoName, bool rndRepo=false,
                QHash<QString, QString> repoParameters=QHash<QString, QString>(),
                QHash<QString, QString> parametersOverride=QHash<QString, QString>());

};

#endif
