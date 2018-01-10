/**
 * @file ssu.h
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#ifndef _Ssu_H
#define _Ssu_H

#include <QDateTime>
#include <QObject>
#include <QDebug>

class QNetworkAccessManager;
class QNetworkReply;
class QDomDocument;

class Ssu: public QObject
{
    Q_OBJECT

    friend class UrlResolverTest;

    Q_PROPERTY(bool registered READ isRegistered NOTIFY registrationStatusChanged)
    Q_PROPERTY(DeviceModeFlags deviceMode READ deviceMode CONSTANT)
    Q_PROPERTY(QString domain READ domain CONSTANT)
public:
    /**
     * Filters to control the output of the repository lookup methods
     */
    enum RepoFilter {
        NoFilter,                 ///< All repositories (global + user)
        UserFilter,               ///< Only user configured repositories
        BoardFilter,              ///< Only global repositories, with user blacklist ignored
        BoardFilterUserBlacklist, ///< Only global repositories, with user blacklist applied
    };

    /**
     * List of possible device modes
     *
     * ReleaseMode is defined to make a switch to allowing RnD and Release
     * repositories on a device at the same time easy, if ever needed. Right
     * now any mode where RndMode is not set is treated as ReleaseMode.
     *
     * If both UpdateMode and AppInstallMode are specified the device behaves
     * like in AppInstallMode.
     */
    enum DeviceMode {
        DisableRepoManager = 0x1,   ///< Disable automagic repository management
        RndMode            = 0x2,   ///< Enable RnD mode for device
        ReleaseMode        = 0x4,   ///< Enable Release mode
        LenientMode        = 0x8,   ///< Disable strict mode (i.e., keep unmanaged repositories)
        UpdateMode         = 0x10,  ///< Do repo isolation and similar bits important for updating devices
        AppInstallMode     = 0x20   ///< Do repo isolation, but keep store repository enabled
    };

    Q_DECLARE_FLAGS(DeviceModeFlags, DeviceMode)
    Q_FLAGS(DeviceModeFlags)

    /**
     * A list of types ssu provides shiny values suitable for displaying
     */
    enum DisplayType {
        DeviceManufacturer = 0,     ///< Manufacturer, like ACME Corp. Board mappings key "deviceManufacturer"
        DeviceModel,                ///< Marketed device name, like Pogoblaster 3000. Board mappings key "prettyModel"
        DeviceDesignation,          ///< Type designation, like NCC-1701. Beard mappings key "deviceDesignation"
    };

    /**
     * Edit modes for variables containing bitmasks
     */
    enum EditMode {
        Replace = 0x1, ///< Replace the old value with the new one
        Add     = 0x2, ///< Make sure the given value is set in the bitmask
        Remove  = 0x4, ///< Make sure the given value is not set in the bitmask
    };

    /**
     * Return codes to signal success or error conditions
     */
    enum ReturnValue {
        Success = 0,
        ErrUpdateMode = -10,
    };

    Ssu();
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
     * Returns if the last operation was successful
     * @retval true last operation was successful
     * @retval false last operation failed, you should check lastError() for details
     */
    Q_INVOKABLE bool error();
    /**
     * Return an error message for the last error encountered. The message
     * will not be cleared, check error() to see if the last operation was
     * successful.
     */
    Q_INVOKABLE QString lastError();
    /**
     * Resolve a repository url
     * @return the repository URL on success, an empty string on error
     */
    QString repoUrl(const QString &repoName, bool rndRepo = false,
                    QHash<QString, QString> repoParameters = QHash<QString, QString>(),
                    QHash<QString, QString> parametersOverride = QHash<QString, QString>());
    /**
     * Unregister a device. This will clean all registration data from a device,
     * though will not touch the information on ssu server; the information there
     * has to be manually cleaned for a device we don't own anymore, but will be
     * overwritten next time the device gets registered
     */
    Q_INVOKABLE void unregister();

    // wrappers around SsuCoreConfig
    // not all of those belong into SsuCoreConfig, but will go there
    // in the first phase of refactoring

    /// See SsuCoreConfig::flavour
    Q_INVOKABLE QString flavour();
    /// See SsuCoreConfig::deviceMode
    Q_INVOKABLE DeviceModeFlags deviceMode();
    /// See SsuCoreConfig::domain; returns printable version
    Q_INVOKABLE QString domain();
    /// See SsuCoreConfig::isRegistered
    Q_INVOKABLE bool isRegistered();
    /// See SsuCoreConfig::lastCredentialsUpdate
    Q_INVOKABLE QDateTime lastCredentialsUpdate();
    /// See SsuCoreConfig::release
    Q_INVOKABLE QString release(bool rnd = false);
    /// See SsuCoreConfig::setDeviceMode
    Q_INVOKABLE void setDeviceMode(DeviceModeFlags mode, enum EditMode editMode = Replace);
    /// See SsuCoreConfig::setFlavour
    Q_INVOKABLE void setFlavour(const QString &flavour);
    /// See SsuCoreConfig::setRelease
    Q_INVOKABLE void setRelease(const QString &release, bool rnd = false);
    /// See SsuCoreConfig::setDomain
    Q_INVOKABLE void setDomain(const QString &domain);
    /// See SsuCoreConfig::useSslVerify
    Q_INVOKABLE bool useSslVerify();

private:
    QString errorString;
    bool errorFlag;
    QNetworkAccessManager *manager;
    int pendingRequests;
    bool registerDevice(QDomDocument *response);
    bool setCredentials(QDomDocument *response);
    bool verifyResponse(QDomDocument *response);
    void storeAuthorizedKeys(const QByteArray &data);

private slots:
    void requestFinished(QNetworkReply *reply);
    /**
     * Set errorString returned by lastError to errorMessage, set
     * errorFlag returned by error() to true, and emit done()
     */
    void setError(const QString &errorMessage);

public slots:
    /**
     * Attempt RND device registration, using @a username and @a password supplied
     * @param username Jolla username
     * @param password Jolla password
     *
     * When the operation has finished the done() signal will be sent. You can call
     * error() to check if an error occured, and use lastError() to retrieve the last
     * error message.
     */
    void sendRegistration(const QString &username, const QString &password);
    /**
     * Try to update the RND repository credentials. The device needs to be registered
     * for this to work. updateCredentials remembers the time of the last credentials
     * update, and skips updating if only little time has elapsed since the last update.
     * An update may be forced by setting @a force to true
     * @param force force credentials updating
     *
     * When the operation has finished the done() signal will be sent. You can call
     * error() to check if an error occured, and use lastError() to retrieve the last
     * error message.
     */
    void updateCredentials(bool force = false);
    /**
     * Try to update credentials for (Jolla) store
     */
    void updateStoreCredentials();

signals:
    /**
     * Emitted after an asynchronous operation finished
     */
    void done();
    /**
     * Emitted after the devices registration status has changed
     */
    void registrationStatusChanged();
    void credentialsChanged();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Ssu::DeviceModeFlags)

#endif
