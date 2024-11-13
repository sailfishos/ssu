/**
 * @file ssucoreconfig_p.h
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#ifndef _SSUCORECONFIG_P_H
#define _SSUCORECONFIG_P_H

#include <QObject>
#include "ssusettings_p.h"
#include "ssu.h"

#ifndef SSU_CONFIGURATION
/// Path to the main ssu configuration file
#define SSU_CONFIGURATION "/etc/ssu/ssu.ini"
#endif

#ifndef SSU_DEFAULT_CONFIGURATION
/// Path to the main ssu configuration file
#define SSU_DEFAULT_CONFIGURATION "/usr/share/ssu/ssu-defaults.ini"
#endif

class QDBusConnection;

class SsuCoreConfig: public SsuSettings
{
    Q_OBJECT

public:
    SsuCoreConfig();
    virtual ~SsuCoreConfig();

    static SsuCoreConfig *instance();

    /**
     * Find a username/password pair for the given scope
     * @return a QPair with username and password, or an empty QPair if scope is invalid
     */
    QPair<QString, QString> credentials(const QString &scope);

    /**
     * Get the scope for a repository, taking into account different scopes for
     * release and RnD repositories
     *
     * Please note that variable scope is not yet implemented -- one default scope is
     * read from the configuration file.
     *
     * @return a string containing the scope; it can be used to look up login credentials using  credentials()
     */
    QString credentialsScope(const QString &repoName, bool rndRepo = false);

    /**
     * Return the URL for which credentials scope is valid
     */
    QString credentialsUrl(const QString &scope);

    /**
     * Get the current flavour when RnD repositories are used
     * @return current flavour (usually something like testing, release, ..)
     */
    Q_INVOKABLE QString flavour();

    /**
     * Get the current mode bits for the device
     */
    Q_INVOKABLE Ssu::DeviceModeFlags deviceMode();

    /**
     * Get the current domain used in registration
     * Internally - in the domain is replaced by :, if you need
     * to print the domain name set pretty to true
     * @return domain, or "" if not set
     */
    Q_INVOKABLE QString domain(bool pretty = false);

    /**
     * Get the current brand stored in the SSU_CONFIGURATION
     * @return brand, or "" if not set
     */
    Q_INVOKABLE QString brand();

    /**
     * Return devices RND registration status
     * @retval true device is registered
     * @retval false device is not registered
     */
    Q_INVOKABLE bool isRegistered();

    /**
     * Return the date/time when the credentials to access internal
     * ssu servers were updated the last time
     */
    Q_INVOKABLE QDateTime lastCredentialsUpdate();

    /**
     * Return the release version string for either a release, or a RnD snapshot
     */
    Q_INVOKABLE QString release(bool rnd = false);

    /**
     * Set mode bits for the device
     */
    Q_INVOKABLE void setDeviceMode(Ssu::DeviceModeFlags mode, enum Ssu::EditMode editMode = Ssu::Replace);

    /**
     * Set the flavour used when resolving RND repositories
     */
    Q_INVOKABLE void setFlavour(const QString &flavour);

    /**
     * Set the release version string for either a release, or a RnD snapshot
     */
    Q_INVOKABLE void setRelease(const QString &release, bool rnd = false);

    /**
     * Set the domain string (usually something like nemo, jolla, ..)
     */
    Q_INVOKABLE void setDomain(const QString &domain);

    /**
     * Return configuration settings regarding ssl verification
     * @retval true SSL verification must be used; that's the default if not configured
     * @retval false SSL verification should be disabled
     */
    Q_INVOKABLE bool useSslVerify();

    /**
     * Return a DBus connection object connected to the session bus of the active user
     */
    static QDBusConnection userSessionBus();

private:
    Q_DISABLE_COPY(SsuCoreConfig)
};

#endif
