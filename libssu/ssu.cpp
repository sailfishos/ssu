/**
 * @file ssu.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#include <QtXml/QDomDocument>

#include "ssu.h"
#include "ssulog.h"
#include "ssuvariables.h"

#include "../constants.h"

Ssu::Ssu(QString fallbackLog): QObject(){
  errorFlag = false;
  fallbackLogPath = fallbackLog;
  pendingRequests = 0;

#ifdef SSUCONFHACK
  // dirty hack to make sure we can write to the configuration
  // this is currently required since there's no global gconf,
  // and we migth not yet have users on bootstrap
  QFileInfo settingsInfo(SSU_CONFIGURATION);
  if (settingsInfo.groupId() != SSU_GROUP_ID ||
      !settingsInfo.permission(QFile::WriteGroup)){
    QProcess proc;
    proc.start("/usr/bin/ssuconfperm");
    proc.waitForFinished();
  }
#endif

  settings = new SsuSettings(SSU_CONFIGURATION, QSettings::IniFormat, SSU_DEFAULT_CONFIGURATION);
  repoSettings = new QSettings(SSU_REPO_CONFIGURATION, QSettings::IniFormat);

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

QPair<QString, QString> Ssu::credentials(QString scope){
  QPair<QString, QString> ret;
  settings->beginGroup("credentials-" + scope);
  ret.first = settings->value("username").toString();
  ret.second = settings->value("password").toString();
  settings->endGroup();
  return ret;
}

QString Ssu::credentialsScope(QString repoName, bool rndRepo){
  if (settings->contains("credentials-scope"))
    return settings->value("credentials-scope").toString();
  else
    return "your-configuration-is-broken-and-does-not-contain-credentials-scope";
}

QString Ssu::credentialsUrl(QString scope){
  if (settings->contains("credentials-url-" + scope))
    return settings->value("credentials-url-" + scope).toString();
  else
    return "your-configuration-is-broken-and-does-not-contain-credentials-url-for-" + scope;
}

bool Ssu::error(){
  return errorFlag;
}

QString Ssu::flavour(){
  if (settings->contains("flavour"))
    return settings->value("flavour").toString();
  else
    return "release";
}

QString Ssu::domain(){
  if (settings->contains("domain"))
    return settings->value("domain").toString();
  else
    return "";
}

bool Ssu::isRegistered(){
  if (!settings->contains("privateKey"))
    return false;
  if (!settings->contains("certificate"))
    return false;
  return settings->value("registered").toBool();
}

QDateTime Ssu::lastCredentialsUpdate(){
  return settings->value("lastCredentialsUpdate").toDateTime();
}

QString Ssu::lastError(){
  return errorString;
}

bool Ssu::registerDevice(QDomDocument *response){
  QString certificateString = response->elementsByTagName("certificate").at(0).toElement().text();
  QSslCertificate certificate(certificateString.toAscii());
  SsuLog *ssuLog = SsuLog::instance();

  if (certificate.isNull()){
    // make sure device is in unregistered state on failed registration
    settings->setValue("registered", false);
    setError("Certificate is invalid");
    return false;
  } else
    settings->setValue("certificate", certificate.toPem());

  QString privateKeyString = response->elementsByTagName("privateKey").at(0).toElement().text();
  QSslKey privateKey(privateKeyString.toAscii(), QSsl::Rsa);

  if (privateKey.isNull()){
    settings->setValue("registered", false);
    setError("Private key is invalid");
    return false;
  } else
    settings->setValue("privateKey", privateKey.toPem());

  // oldUser is just for reference purposes, in case we want to notify
  // about owner changes for the device
  QString oldUser = response->elementsByTagName("user").at(0).toElement().text();
  ssuLog->print(LOG_DEBUG, QString("Old user for your device was: %1").arg(oldUser));

  // if we came that far everything required for device registration is done
  settings->setValue("registered", true);
  settings->sync();
  emit registrationStatusChanged();
  return true;
}

QString Ssu::release(bool rnd){
  if (rnd)
    return settings->value("rndRelease").toString();
  else
    return settings->value("release").toString();
}

// RND repos have flavour (devel, testing, release), and release (latest, next)
// Release repos only have release (latest, next, version number)
QString Ssu::repoUrl(QString repoName, bool rndRepo, QHash<QString, QString> repoParameters){
  QString r;
  QStringList configSections;
  SsuVariables var;

  errorFlag = false;

  // fill in all arbitrary variables from ssu.ini
  var.resolveSection(settings, "repository-url-variables", &repoParameters);

  // add/overwrite some of the variables with sane ones
  if (rndRepo){
    repoParameters.insert("flavour", repoSettings->value(flavour()+"-flavour/flavour-pattern").toString());
    repoParameters.insert("flavourPattern", repoSettings->value(flavour()+"-flavour/flavour-pattern").toString());
    repoParameters.insert("flavourName", flavour());
    configSections << flavour()+"-flavour" << "rnd" << "all";

    // Make it possible to give any values with the flavour as well.
    // These values can be overridden later with domain if needed.
    var.resolveSection(repoSettings, flavour()+"-flavour", &repoParameters);
  } else {
    configSections << "release" << "all";
  }

  repoParameters.insert("release", release(rndRepo));

  if (!repoParameters.contains("debugSplit"))
    repoParameters.insert("debugSplit", "packages");

  if (!repoParameters.contains("arch"))
    repoParameters.insert("arch", settings->value("arch").toString());

  repoParameters.insert("adaptation", settings->value("adaptation").toString());
  repoParameters.insert("deviceFamily", deviceInfo.deviceFamily());
  repoParameters.insert("deviceModel", deviceInfo.deviceModel());

  QStringList keys;
  keys << "chip" << "adaptation" << "vendor";

  foreach(QString key, keys){
    QString value;
    if (deviceInfo.getValue(key,value))
      repoParameters.insert(key, value);
  }

  // Domain variables
  // first read all variables from default-domain
  var.resolveSection(repoSettings, "default-domain", &repoParameters);

  // then overwrite with domain specific things if that block is available
  var.resolveSection(repoSettings, domain()+"-domain", &repoParameters);

  if (settings->contains("repository-urls/" + repoName))
    r = settings->value("repository-urls/" + repoName).toString();
  else {
    foreach (const QString &section, configSections){
      repoSettings->beginGroup(section);
      if (repoSettings->contains(repoName)){
        r = repoSettings->value(repoName).toString();
        repoSettings->endGroup();
        break;
      }
      repoSettings->endGroup();
    }
  }

  return var.resolveString(r, &repoParameters);
}

void Ssu::requestFinished(QNetworkReply *reply){
  QSslConfiguration sslConfiguration = reply->sslConfiguration();
  SsuLog *ssuLog = SsuLog::instance();

  ssuLog->print(LOG_DEBUG, QString("Certificate used was issued for '%1' by '%2'. Complete chain:")
               .arg(sslConfiguration.peerCertificate().subjectInfo(QSslCertificate::CommonName))
               .arg(sslConfiguration.peerCertificate().issuerInfo(QSslCertificate::CommonName)));

  foreach (const QSslCertificate cert, sslConfiguration.peerCertificateChain()){
    ssuLog->print(LOG_DEBUG, QString("-> %1").arg(cert.subjectInfo(QSslCertificate::CommonName)));
  }

  // what sucks more, this or goto?
  do {
    if (settings->contains("home-url")){
      QString homeUrl = settings->value("home-url").toString().arg("");
      homeUrl.remove(QRegExp("//+$"));
      QNetworkRequest request = reply->request();

      if (request.url().toString().startsWith(homeUrl, Qt::CaseInsensitive)){
        // we don't care about errors on download request
        if (reply->error() > 0) break;
        QByteArray data = reply->readAll();
        storeAuthorizedKeys(data);
        break;
      }
    }

    if (reply->error() > 0){
      pendingRequests--;
      setError(reply->errorString());
      return;
    } else {
      QByteArray data = reply->readAll();
      qDebug() << "RequestOutput" << data;

      QDomDocument doc;
      QString xmlError;
      if (!doc.setContent(data, &xmlError)){
        pendingRequests--;
        setError(tr("Unable to parse server response (%1)").arg(xmlError));
        return;
      }

      QString action = doc.elementsByTagName("action").at(0).toElement().text();

      if (!verifyResponse(&doc)) break;

      if (action == "register"){
        if (!registerDevice(&doc)) break;
      } else if (action == "credentials"){
        if (!setCredentials(&doc)) break;
      } else {
        pendingRequests--;
        setError(tr("Response to unknown action encountered: %1").arg(action));
        return;
      }
    }
  } while (false);

  pendingRequests--;

  ssuLog->print(LOG_DEBUG, QString("Request finished, pending requests: %1").arg(pendingRequests));
  if (pendingRequests == 0)
    emit done();
}

void Ssu::sendRegistration(QString usernameDomain, QString password){
  errorFlag = false;

  QString ssuCaCertificate, ssuRegisterUrl;
  QString username, domainName;

  SsuLog *ssuLog = SsuLog::instance();

  // Username can include also domain, (user@domain), separate those
  if (usernameDomain.contains('@')) {
      // separate domain/username and set domain
      username = usernameDomain.section('@', 0, 0);
      domainName = usernameDomain.section('@', 1, 1);
      setDomain(domainName);
  } else {
      // No domain defined
      username = usernameDomain;
  }

  if (!settings->contains("ca-certificate")){
    setError("CA certificate for SSU not set (config key 'ca-certificate')");
    return;
  } else
    ssuCaCertificate = settings->value("ca-certificate").toString();

  if (!settings->contains("register-url")){
    ssuRegisterUrl = repoUrl("register-url");
    if (ssuRegisterUrl.isEmpty()){
      setError("URL for SSU registration not set (config key 'register-url')");
      return;
    }
  } else
    ssuRegisterUrl = settings->value("register-url").toString();

  QString IMEI = deviceInfo.deviceUid();
  if (IMEI == ""){
    setError("No valid UID available for your device. For phones: is your modem online?");
    return;
  }

  QSslConfiguration sslConfiguration;
  if (!useSslVerify())
    sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);

  sslConfiguration.setCaCertificates(QSslCertificate::fromPath(ssuCaCertificate));

  QNetworkRequest request;
  request.setUrl(QUrl(QString(ssuRegisterUrl)
                      .arg(IMEI)
                   ));
  request.setSslConfiguration(sslConfiguration);
  request.setRawHeader("Authorization", "Basic " +
                       QByteArray(QString("%1:%2")
                                  .arg(username).arg(password)
                                  .toAscii()).toBase64());
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

  QUrl form;
  form.addQueryItem("protocolVersion", SSU_PROTOCOL_VERSION);
  form.addQueryItem("deviceModel", deviceInfo.deviceModel());
  if (!domain().isEmpty()){
    form.addQueryItem("domain", domain());
  }

  qDebug() << "Sending request to " << request.url();
  qDebug() << form.encodedQueryItems();

  QNetworkReply *reply;

  pendingRequests++;
  reply = manager->post(request, form.encodedQuery());
  // we could expose downloadProgress() from reply in case we want progress info

  QString homeUrl = settings->value("home-url").toString().arg(username);
  if (!homeUrl.isEmpty()){
    // clear header, the other request bits are reusable
    request.setHeader(QNetworkRequest::ContentTypeHeader, 0);
    request.setUrl(homeUrl + "/authorized_keys");
    ssuLog->print(LOG_DEBUG, QString("Trying to get SSH keys from %1").arg(request.url().toString()));
    pendingRequests++;
    manager->get(request);
  }
}

bool Ssu::setCredentials(QDomDocument *response){
  // generate list with all scopes for generic section, add sections
  QDomNodeList credentialsList = response->elementsByTagName("credentials");
  QStringList credentialScopes;
  for (int i=0;i<credentialsList.size();i++){
    QDomNode node = credentialsList.at(i);
    QString scope;

    QDomNamedNodeMap attributes = node.attributes();
    if (attributes.contains("scope")){
      scope = attributes.namedItem("scope").toAttr().value();
    } else {
      setError(tr("Credentials element does not have scope"));
      return false;
    }

    if (node.hasChildNodes()){
      QDomElement username = node.firstChildElement("username");
      QDomElement password = node.firstChildElement("password");
      if (username.isNull() || password.isNull()){
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
      setError("");
      return false;
    }
  }
  settings->setValue("credentialScopes", credentialScopes);
  settings->setValue("lastCredentialsUpdate", QDateTime::currentDateTime());
  settings->sync();
  emit credentialsChanged();

  return true;
}

void Ssu::setError(QString errorMessage){
  errorFlag = true;
  errorString = errorMessage;

  SsuLog *ssuLog = SsuLog::instance();

  // dump error message to systemd journal for easier debugging
  ssuLog->print(LOG_WARNING, errorMessage);

  // assume that we don't even need to wait for other pending requests,
  // and just die. This is only relevant for CLI, which will exit after done()
  emit done();
}

void Ssu::setFlavour(QString flavour){
  settings->setValue("flavour", flavour);
  settings->sync();
  emit flavourChanged();
}

void Ssu::setRelease(QString release, bool rnd){
  if (rnd)
    settings->setValue("rndRelease", release);
  else
    settings->setValue("release", release);
  settings->sync();
}

void Ssu::setDomain(QString domain){
  settings->setValue("domain", domain);
  settings->sync();
}

void Ssu::storeAuthorizedKeys(QByteArray data){
  QDir dir;

  // only set the key for unprivileged users
  if (getuid() < 1000) return;

  if (dir.exists(dir.homePath() + "/.ssh/authorized_keys"))
    return;

  if (!dir.exists(dir.homePath() + "/.ssh"))
    if (!dir.mkdir(dir.homePath() + "/.ssh")) return;

  QFile::setPermissions(dir.homePath() + "/.ssh",
                        QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);

  QFile authorizedKeys(dir.homePath() + "/.ssh/authorized_keys");
  authorizedKeys.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
  authorizedKeys.setPermissions(QFile::ReadOwner | QFile::WriteOwner);
  QTextStream out(&authorizedKeys);
  out << data;
  out.flush();
  authorizedKeys.close();
}

void Ssu::updateCredentials(bool force){
  errorFlag = false;

  SsuLog *ssuLog = SsuLog::instance();

  if (deviceInfo.deviceUid() == ""){
    setError("No valid UID available for your device. For phones: is your modem online?");
    return;
  }

  QString ssuCaCertificate, ssuCredentialsUrl;
  if (!settings->contains("ca-certificate")){
    setError("CA certificate for SSU not set (config key 'ca-certificate')");
    return;
  } else
    ssuCaCertificate = settings->value("ca-certificate").toString();

  if (!settings->contains("credentials-url")){
    ssuCredentialsUrl = repoUrl("credentials-url");
    if (ssuCredentialsUrl.isEmpty()){
      setError("URL for credentials update not set (config key 'credentials-url')");
      return;
    }
  } else
    ssuCredentialsUrl = settings->value("credentials-url").toString();

  if (!isRegistered()){
    setError("Device is not registered.");
    return;
  }

  if (!force){
    // skip updating if the last update was less than 30 minutes ago
    QDateTime now = QDateTime::currentDateTime();

    if (settings->contains("lastCredentialsUpdate")){
      QDateTime last = settings->value("lastCredentialsUpdate").toDateTime();
      if (last >= now.addSecs(-1800)){
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

bool Ssu::useSslVerify(){
  if (settings->contains("ssl-verify"))
    return settings->value("ssl-verify").toBool();
  else
    return true;
}

void Ssu::unregister(){
  settings->setValue("privateKey", "");
  settings->setValue("certificate", "");
  settings->setValue("registered", false);
  settings->sync();
  emit registrationStatusChanged();
}

bool Ssu::verifyResponse(QDomDocument *response){
  QString action = response->elementsByTagName("action").at(0).toElement().text();
  QString deviceId = response->elementsByTagName("deviceId").at(0).toElement().text();
  QString protocolVersion = response->elementsByTagName("protocolVersion").at(0).toElement().text();
  // compare device ids

  if (protocolVersion != SSU_PROTOCOL_VERSION){
    setError(
      tr("Response has unsupported protocol version %1, client requires version %2")
      .arg(protocolVersion)
      .arg(SSU_PROTOCOL_VERSION)
      );
    return false;
  }

  return true;
}
