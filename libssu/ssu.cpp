/**
 * @file ssu.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#include <QtNetwork>
#include <QtXml/QDomDocument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QUrlQuery>

#include <getdef.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include "ssu.h"
#include "sandbox_p.h"
#include "ssulog_p.h"
#include "ssuvariables_p.h"
#include "ssucoreconfig_p.h"
#include "ssurepomanager.h"
#include "ssudeviceinfo.h"

#include "../constants.h"

#define SSU_NETWORK_REQUEST_DOMAIN_DATA (static_cast<QNetworkRequest::Attribute>(QNetworkRequest::User + 1))

static void restoreUid()
{
    if (getuid() == 0) {
        seteuid(0);
        setegid(0);
    }
}

Ssu::Ssu()
    : QObject()
{
    errorFlag = false;
    pendingRequests = 0;

#ifdef SSUCONFHACK
    // dirty hack to make sure we can write to the configuration
    // this is currently required since there's no global gconf,
    // and we migth not yet have users on bootstrap
    QFileInfo settingsInfo(SSU_CONFIGURATION);
    if (settingsInfo.groupId() != SSU_GROUP_ID ||
            !settingsInfo.permission(QFile::WriteGroup)) {
        QProcess proc;
        proc.start("/usr/bin/ssuconfperm");
        proc.waitForFinished();
    }
#endif

    SsuCoreConfig *settings = SsuCoreConfig::instance();

#ifdef TARGET_ARCH
    if (!settings->contains("arch"))
        settings->setValue("arch", TARGET_ARCH);
#else
// FIXME, try to guess a matching architecture
#warning "TARGET_ARCH not defined"
#endif
    settings->sync();



    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply *)),
            SLOT(requestFinished(QNetworkReply *)));
}

// FIXME, the whole credentials stuff needs reworking
// should probably be part of repo handling instead of core configuration
QPair<QString, QString> Ssu::credentials(const QString &scope)
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    return settings->credentials(scope);
}

QString Ssu::credentialsScope(const QString &repoName, bool rndRepo)
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    SsuSettings repoSettings(SSU_REPO_CONFIGURATION, SSU_REPO_CONFIGURATION_DIR);

    // hardcoded magic for doing special privileges store repositories
    if (repoName == "store" || repoName.startsWith("store-c-"))
        return "store";

    // check if some repos are marked for using store-credentials
    // in current domain, checking first for rnd/release specific
    // settings, and if not found in generic settings
    QString storeAuthReposKey = QString("store-auth-repos-%1")
                                .arg(rndRepo ? "rnd" : "release");
    QStringList storeAuthRepos =
        SsuVariables::variable(&repoSettings,
                               domain() + "-domain",
                               storeAuthReposKey).toStringList();

    if (storeAuthRepos.empty())
        storeAuthRepos =
            SsuVariables::variable(&repoSettings,
                                   domain() + "-domain",
                                   "store-auth-repos").toStringList();

    if (storeAuthRepos.contains(repoName))
        return "store";

    // If we defined explicitly what credentials to use with which secure domain
    // use those.
    QHash<QString, QString> secureDomainAuth;
    SsuVariables::variableSection(&repoSettings, "secure-domain-auth", &secureDomainAuth);
    QHashIterator<QString, QString> i(secureDomainAuth);
    while (i.hasNext()) {
        i.next();
        if (repoUrl(repoName, rndRepo).contains(i.key()) && !i.value().isEmpty()) {
            return i.value();
        }
    }

    return settings->credentialsScope(repoName, rndRepo);
}

QString Ssu::credentialsUrl(const QString &scope)
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    return settings->credentialsUrl(scope);
}

bool Ssu::error()
{
    return errorFlag;
}

// Wrappers around SsuCoreConfig
QString Ssu::flavour()
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    return settings->flavour();
}

Ssu::DeviceModeFlags Ssu::deviceMode()
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    return settings->deviceMode();
}

QString Ssu::domain()
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    return settings->domain(true);
}

QString Ssu::brand() {
  SsuCoreConfig *settings = SsuCoreConfig::instance();
  return settings->brand();
}

bool Ssu::isRegistered()
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    return settings->isRegistered();
}

QDateTime Ssu::lastCredentialsUpdate()
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    return settings->lastCredentialsUpdate();
}

QString Ssu::release(bool rnd)
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    return settings->release(rnd);
}

void Ssu::setDeviceMode(Ssu::DeviceModeFlags mode, enum Ssu::EditMode editMode)
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    settings->setDeviceMode(mode, editMode);
}

void Ssu::setFlavour(const QString &flavour)
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    settings->setFlavour(flavour);
}

void Ssu::setRelease(const QString &release, bool rnd)
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    settings->setRelease(release, rnd);
}

void Ssu::setDomain(const QString &domain)
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    settings->setDomain(domain);
}

bool Ssu::useSslVerify()
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    return settings->useSslVerify();
}

//...



QString Ssu::lastError()
{
    return errorString;
}

bool Ssu::registerDevice(QDomDocument *response)
{
    QString certificateString = response->elementsByTagName("certificate").at(0).toElement().text();
    QSslCertificate certificate(certificateString.toLatin1());
    SsuLog *ssuLog = SsuLog::instance();
    SsuCoreConfig *settings = SsuCoreConfig::instance();

    if (certificate.isNull()) {
        // make sure device is in unregistered state on failed registration
        settings->setValue("registered", false);
        setError("Certificate is invalid");
        return false;
    } else {
        settings->setValue("certificate", certificate.toPem());
    }

    QString privateKeyString = response->elementsByTagName("privateKey").at(0).toElement().text();
    QSslKey privateKey(privateKeyString.toLatin1(), QSsl::Rsa);

    if (privateKey.isNull()) {
        settings->setValue("registered", false);
        setError("Private key is invalid");
        return false;
    } else {
        settings->setValue("privateKey", privateKey.toPem());
    }

    // oldUser is just for reference purposes, in case we want to notify
    // about owner changes for the device
    QString oldUser = response->elementsByTagName("user").at(0).toElement().text();
    ssuLog->print(LOG_DEBUG, QString("Old user for your device was: %1").arg(oldUser));

    // if we came that far everything required for device registration is done
    settings->setValue("registered", true);
    settings->sync();

    if (!settings->isWritable()) {
        setError("Configuration is not writable, device registration failed.");
        return false;
    }

    emit registrationStatusChanged();
    return true;
}

// RND repos have flavour (devel, testing, release), and release (latest, next)
// Release repos only have release (latest, next, version number)
QString Ssu::repoUrl(const QString &repoName, bool rndRepo,
                     QHash<QString, QString> repoParameters,
                     QHash<QString, QString> parametersOverride)
{
    SsuRepoManager manager;
    return manager.url(repoName, rndRepo, repoParameters, parametersOverride);
}

void Ssu::requestFinished(QNetworkReply *reply)
{
    QSslConfiguration sslConfiguration = reply->sslConfiguration();
    SsuLog *ssuLog = SsuLog::instance();
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    QNetworkRequest request = reply->request();
    QVariant originalDomainVariant = request.attribute(SSU_NETWORK_REQUEST_DOMAIN_DATA);

    ssuLog->print(LOG_DEBUG, QString("Certificate used was issued for '%1' by '%2'. Complete chain:")
                  .arg(sslConfiguration.peerCertificate().subjectInfo(QSslCertificate::CommonName).join(""))
                  .arg(sslConfiguration.peerCertificate().issuerInfo(QSslCertificate::CommonName).join("")));

    foreach (const QSslCertificate cert, sslConfiguration.peerCertificateChain()) {
        ssuLog->print(LOG_DEBUG, QString("-> %1").arg(cert.subjectInfo(QSslCertificate::CommonName).join("")));
    }

    pendingRequests--;

    QString action;
    QByteArray data;
    QDomDocument doc;
    QString xmlError;

    /// @TODO: indicate that the device is not registered if there's a 404 on credentials update url
    if (settings->contains("home-url")) {
        QString homeUrl = settings->value("home-url").toString().arg(QString());
        homeUrl.remove(QRegExp("//+$"));

        if (request.url().toString().startsWith(homeUrl, Qt::CaseInsensitive)) {
            // we don't care about errors on download request
            if (reply->error() == 0) {
                QByteArray data = reply->readAll();
                storeAuthorizedKeys(data);
            }

            goto success;
        }
    }

    if (reply->error() > 0) {
        setError(reply->errorString());
        goto failure;
    }

    data = reply->readAll();
    ssuLog->print(LOG_DEBUG, QString("RequestOutput %1")
                  .arg(data.data()));

    if (!doc.setContent(data, &xmlError)) {
        setError(tr("Unable to parse server response (%1)").arg(xmlError));
        goto failure;
    }

    action = doc.elementsByTagName("action").at(0).toElement().text();

    if (!verifyResponse(&doc)) {
        goto failure;
    }

    ssuLog->print(LOG_DEBUG, QString("Handling request of type %1")
                  .arg(action));
    if (action == "register") {
        if (registerDevice(&doc)) {
            goto success;
        }
    } else if (action == "credentials") {
        if (setCredentials(&doc)) {
            goto success;
        }
    } else {
        setError(tr("Response to unknown action encountered: %1").arg(action));
    }

failure:
    // Restore the original domain in case of failures with the registration
    if (!originalDomainVariant.isNull()) {
        QString originalDomain = originalDomainVariant.toString();
        ssuLog->print(LOG_DEBUG, QString("Restoring domain on error: '%1'").arg(originalDomain));
        setDomain(originalDomain);
    }

    // Fall through to cleanup handling in success from failure label
success:
    ssuLog->print(LOG_DEBUG, QString("Request finished, pending requests: %1").arg(pendingRequests));
    if (pendingRequests == 0) {
        emit done();
    }
}

void Ssu::sendRegistration(const QString &usernameDomain, const QString &password)
{
    errorFlag = false;

    QString ssuRegisterUrl;
    QString username, domainName;

    SsuLog *ssuLog = SsuLog::instance();
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    SsuDeviceInfo deviceInfo;

    QNetworkRequest request;
    request.setAttribute(SSU_NETWORK_REQUEST_DOMAIN_DATA, domain());
    ssuLog->print(LOG_DEBUG, QString("Saving current domain before request: '%1'").arg(domain()));

    // Username can include also domain, (user@domain), separate those
    if (usernameDomain.contains('@')) {
        // separate domain/username and set domain
        username = usernameDomain.section('@', 0, 0);
        domainName = usernameDomain.section('@', 1, 1);
        setDomain(domainName);
    } else {
        // No domain defined
        username = usernameDomain;
        if (settings->contains("default-rnd-domain"))
            setDomain(settings->value("default-rnd-domain").toString());
    }

    QString ssuCaCertificate = SsuRepoManager::caCertificatePath();
    if (ssuCaCertificate.isEmpty()) {
        setError("CA certificate for ssu not set ('_ca-certificate in domain')");
        return;
    }

    if (!settings->contains("register-url")) {
        ssuRegisterUrl = repoUrl("register-url");
        if (ssuRegisterUrl.isEmpty()) {
            setError("URL for ssu registration not set (config key 'register-url')");
            return;
        }
    } else {
        ssuRegisterUrl = settings->value("register-url").toString();
    }

    QString IMEI = deviceInfo.deviceUid();
    if (IMEI.isEmpty()) {
        setError("No valid UID available for your device. For phones: is your modem online?");
        return;
    }

    QSslConfiguration sslConfiguration;
    if (!useSslVerify())
        sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);

    sslConfiguration.setCaCertificates(QSslCertificate::fromPath(ssuCaCertificate));

    request.setUrl(QUrl(QString(ssuRegisterUrl)
                        .arg(IMEI)
                       ));
    request.setSslConfiguration(sslConfiguration);
    request.setRawHeader("Authorization", "Basic " +
                         QByteArray(QString("%1:%2")
                                    .arg(username).arg(password)
                                    .toLatin1()).toBase64());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrl form;

    QUrlQuery q;
    q.addQueryItem("protocolVersion", SSU_PROTOCOL_VERSION);
    q.addQueryItem("deviceModel", deviceInfo.deviceModel());
    if (!domain().isEmpty()) {
        q.addQueryItem("domain", domain());
    }

    form.setQuery(q);

    ssuLog->print(LOG_DEBUG, QString("Sending request to %1")
                  .arg(request.url().url()));

    pendingRequests++;
    manager->post(request, form.query(QUrl::FullyEncoded).toStdString().c_str());
    // we could expose downloadProgress() from reply in case we want progress info

    QString homeUrl = settings->value("home-url").toString().arg(username);
    if (!homeUrl.isEmpty()) {
        // clear header, the other request bits are reusable
        request.setHeader(QNetworkRequest::ContentTypeHeader, 0);
        request.setUrl(homeUrl + "/authorized_keys");
        ssuLog->print(LOG_DEBUG, QString("Trying to get SSH keys from %1").arg(request.url().toString()));
        pendingRequests++;
        manager->get(request);
    }
}

bool Ssu::setCredentials(QDomDocument *response)
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    // generate list with all scopes for generic section, add sections
    QDomNodeList credentialsList = response->elementsByTagName("credentials");
    QStringList credentialScopes;
    for (int i = 0; i < credentialsList.size(); i++) {
        QDomNode node = credentialsList.at(i);
        QString scope;

        QDomNamedNodeMap attributes = node.attributes();
        if (attributes.contains("scope")) {
            scope = attributes.namedItem("scope").toAttr().value();
        } else {
            setError(tr("Credentials element does not have scope"));
            return false;
        }

        if (node.hasChildNodes()) {
            QDomElement username = node.firstChildElement("username");
            QDomElement password = node.firstChildElement("password");
            if (username.isNull() || password.isNull()) {
                setError(tr("Username and/or password not set"));
                return false;
            } else {
                settings->beginGroup("credentials-" + scope);
                settings->setValue("username", username.text());
                settings->setValue("password", password.text());
                settings->endGroup();
                settings->sync();
                credentialScopes.append(scope);
            }
        } else {
            setError(QString());
            return false;
        }
    }
    settings->setValue("credentialScopes", credentialScopes);
    settings->setValue("lastCredentialsUpdate", QDateTime::currentDateTime());
    settings->sync();
    emit credentialsChanged();

    return true;
}

void Ssu::setError(const QString &errorMessage)
{
    errorFlag = true;
    errorString = errorMessage;

    SsuLog *ssuLog = SsuLog::instance();

    // dump error message to systemd journal for easier debugging
    ssuLog->print(LOG_WARNING, errorMessage);

    // assume that we don't even need to wait for other pending requests,
    // and just die. This is only relevant for CLI, which will exit after done()
    emit done();
}

void Ssu::storeAuthorizedKeys(const QByteArray &data)
{
    QDir dir;
    SsuLog *ssuLog = SsuLog::instance();

    int uid_min = getdef_num("UID_MIN", -1);
    QString homePath;

    if (getuid() >= static_cast<uid_t>(uid_min)) {
        homePath = dir.homePath();
    } else if (getuid() == 0) {
        // place authorized_keys in the default users home when run with uid0
        struct passwd *pw = getpwuid(uid_min);
        if (pw == NULL) {
            ssuLog->print(LOG_DEBUG, QString("Unable to find password entry for uid %1")
                          .arg(uid_min));
            return;
        }

        //homePath = QString(pw->pw_dir);
        homePath = pw->pw_dir;

        // use users uid/gid for creating the directories and files
        setegid(pw->pw_gid);
        seteuid(uid_min);
        ssuLog->print(LOG_DEBUG, QString("Dropping to %1/%2 for writing authorized keys")
                      .arg(uid_min)
                      .arg(pw->pw_gid));
    } else {
        return;
    }

    homePath = Sandbox::map(homePath);

    if (dir.exists(homePath + "/.ssh/authorized_keys")) {
        ssuLog->print(LOG_DEBUG, QString(".ssh/authorized_keys already exists in %1")
                      .arg(homePath));
        restoreUid();
        return;
    }

    if (!dir.exists(homePath + "/.ssh")) {
        if (!dir.mkdir(homePath + "/.ssh")) {
            ssuLog->print(LOG_DEBUG, QString("Unable to create .ssh in %1")
                          .arg(homePath));
            restoreUid();
            return;
        }
    }

    QFile::setPermissions(homePath + "/.ssh",
                          QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);

    QFile authorizedKeys(homePath + "/.ssh/authorized_keys");
    authorizedKeys.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    authorizedKeys.setPermissions(QFile::ReadOwner | QFile::WriteOwner);
    QTextStream out(&authorizedKeys);
    out << data;
    out.flush();
    authorizedKeys.close();

    restoreUid();
}

void Ssu::updateCredentials(bool force)
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    SsuDeviceInfo deviceInfo;
    errorFlag = false;

    SsuLog *ssuLog = SsuLog::instance();

    if (deviceInfo.deviceUid().isEmpty()) {
        setError("No valid UID available for your device. For phones: is your modem online?");
        return;
    }

    QString ssuCaCertificate, ssuCredentialsUrl;
    ssuCaCertificate = SsuRepoManager::caCertificatePath();
    if (ssuCaCertificate.isEmpty()) {
        setError("CA certificate for ssu not set ('_ca-certificate in domain')");
        return;
    }

    if (!settings->contains("credentials-url")) {
        ssuCredentialsUrl = repoUrl("credentials-url");
        if (ssuCredentialsUrl.isEmpty()) {
            setError("URL for credentials update not set (config key 'credentials-url')");
            return;
        }
    } else {
        ssuCredentialsUrl = settings->value("credentials-url").toString();
    }

    if (!isRegistered()) {
        setError("Device is not registered.");
        return;
    }

    if (!force) {
        // skip updating if the last update was less than 30 minutes ago
        QDateTime now = QDateTime::currentDateTime();

        if (settings->contains("lastCredentialsUpdate")) {
            QDateTime last = settings->value("lastCredentialsUpdate").toDateTime();
            if (last >= now.addSecs(-1800)) {
                ssuLog->print(LOG_DEBUG, QString("Skipping credentials update, last update was at %1")
                              .arg(last.toString()));
                emit done();
                return;
            }
        }
    }

    // check when the last update was, decide if an update is required
    QSslConfiguration sslConfiguration;
    if (!useSslVerify())
        sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);

    QSslKey privateKey(settings->value("privateKey").toByteArray(), QSsl::Rsa);
    QSslCertificate certificate(settings->value("certificate").toByteArray());

    QList<QSslCertificate> caCertificates;
    caCertificates << QSslCertificate::fromPath(ssuCaCertificate);
    sslConfiguration.setCaCertificates(caCertificates);

    sslConfiguration.setPrivateKey(privateKey);
    sslConfiguration.setLocalCertificate(certificate);

    QNetworkRequest request;
    request.setUrl(QUrl(ssuCredentialsUrl.arg(deviceInfo.deviceUid())));

    ssuLog->print(LOG_DEBUG, QString("Sending credential update request to %1")
                  .arg(request.url().toString()));
    request.setSslConfiguration(sslConfiguration);

    pendingRequests++;
    manager->get(request);
}

void Ssu::updateStoreCredentials()
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    SsuLog *ssuLog = SsuLog::instance();

    QDBusMessage message = QDBusMessage::createMethodCall("com.jolla.jollastore",
                                                          "/StoreClient",
                                                          "com.jolla.jollastore",
                                                          "storeCredentials");
    QDBusPendingReply<QString, QString> reply = SsuCoreConfig::userSessionBus().asyncCall(message);
    reply.waitForFinished();
    if (reply.isError()) {
        if (settings->value("ignore-credential-errors").toBool() == true) {
            ssuLog->print(LOG_WARNING, QString("Warning: ignore-credential-errors is set, passing auth errors down to libzypp"));
            ssuLog->print(LOG_WARNING, QString("Store credentials not received. %1").arg(reply.error().message()));
        } else {
            setError(QString("Store credentials not received. %1").arg(reply.error().message()));
        }
    } else {
        SsuCoreConfig *settings = SsuCoreConfig::instance();
        settings->beginGroup("credentials-store");
        settings->setValue("username", reply.argumentAt<0>());
        settings->setValue("password", reply.argumentAt<1>());
        settings->endGroup();
        settings->sync();
    }
}

void Ssu::unregister()
{
    SsuCoreConfig *settings = SsuCoreConfig::instance();
    settings->setValue("privateKey", "");
    settings->setValue("certificate", "");
    settings->setValue("registered", false);
    settings->sync();
    emit registrationStatusChanged();
}

bool Ssu::verifyResponse(QDomDocument *response)
{
    QString action = response->elementsByTagName("action").at(0).toElement().text();
    QString deviceId = response->elementsByTagName("deviceId").at(0).toElement().text();
    QString protocolVersion = response->elementsByTagName("protocolVersion").at(0).toElement().text();
    // compare device ids

    if (protocolVersion != SSU_PROTOCOL_VERSION) {
        setError(
            tr("Response has unsupported protocol version %1, client requires version %2")
            .arg(protocolVersion)
            .arg(SSU_PROTOCOL_VERSION)
        );
        return false;
    }

    return true;
}
