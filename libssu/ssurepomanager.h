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

class SsuRepoManager: public QObject {
    Q_OBJECT

  public:
    SsuRepoManager();
    /**
     * Add a repository. Note: Repositories ending with -debuginfo receive special
     * treatment. They'll get saved with the full name to make zypper and the user
     * happy, but internally the -debuginfo will be stripped, and the debugSplit
     * parameter set to debug instead.
     */
    void add(QString repo, QString repoUrl="");
    /**
     * Disable a repository
     */
    void disable(QString repo);
    /**
     * Enable a repository, given it's not disabled by board configuration
     */
    void enable(QString repo);
    /**
     * Remove a repository
     */
    void remove(QString repo);
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
