/**
 * @file ssu.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#include <QSystemDeviceInfo>

#include <QtXml/QDomDocument>
#include "ssu.h"
#include "../constants.h"

QTM_USE_NAMESPACE

Ssu::Ssu(): QObject(){
  errorFlag = false;

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

  settings = new QSettings(SSU_CONFIGURATION, QSettings::IniFormat);
  repoSettings = new QSettings(SSU_REPO_CONFIGURATION, QSettings::IniFormat);

  bool initialized=settings->value("initialized").toBool();
  if (!initialized){
    // FIXME, there's currently no fallback for missing configuration
    settings->setValue("initialized", true);
    settings->setValue("flavour", "testing");
  }

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

QString Ssu::deviceFamily(){
  QString model = deviceModel();

  if (model == "N900")
    return "n900";
  if (model == "N9" || model == "N950")
    return "n950-n9";

  return "UNKNOWN";
}

QString Ssu::deviceModel(){
  QDir dir;
  QFile procCpuinfo;

  if (dir.exists("/mer-sdk-chroot"))
    return "SDK";

  // This part should be handled using QTM::SysInfo, but that's currently broken
  // on Nemo for N9/N950
  procCpuinfo.setFileName("/proc/cpuinfo");
  procCpuinfo.open(QIODevice::ReadOnly | QIODevice::Text);
  if (procCpuinfo.isOpen()){
    QTextStream in(&procCpuinfo);
    QString cpuinfo = in.readAll();
    if (cpuinfo.contains("Nokia RX-51 board"))
      return "N900";
    if (cpuinfo.contains("Nokia RM-680 board"))
      return "N950";
    if (cpuinfo.contains("Nokia RM-696 board"))
      return "N9";
  }

  return "UNKNOWN";
}

QString Ssu::deviceUid(){
  QString IMEI;
  QSystemDeviceInfo devInfo;

  // for all devices where we know that they have an IMEI we can't fall back other UID
  if (deviceFamily() == "n950-n9" || deviceFamily() == "n900")
    return devInfo.imei();

  IMEI = devInfo.imei();
  // this might not be completely unique (or might change on reflash), but works for now
  if (IMEI == "")
    IMEI = devInfo.uniqueDeviceID();
  return IMEI;
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
  qDebug() << "Old user:" << oldUser;

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
  QStringList repoVariables;

  errorFlag = false;

  // fill in all arbitrary variables from ssu.ini
  settings->beginGroup("repository-url-variables");
  repoVariables = settings->allKeys();
  foreach (const QString &key, repoVariables){
    repoParameters.insert(key, settings->value(key).toString());
  }
  settings->endGroup();

  // add/overwrite some of the variables with sane ones
  if (rndRepo){
    repoParameters.insert("flavour", repoSettings->value(flavour()+"-flavour/flavour-pattern").toString());
    repoParameters.insert("release", settings->value("rndRelease").toString());
    configSections << flavour()+"-flavour" << "rnd" << "all";
  } else {
    repoParameters.insert("release", settings->value("release").toString());
    configSections << "release" << "all";
  }

  if (!repoParameters.contains("debugSplit"))
    repoParameters.insert("debugSplit", "packages");

  if (!repoParameters.contains("arch"))
    repoParameters.insert("arch", settings->value("arch").toString());

  repoParameters.insert("adaptation", settings->value("adaptation").toString());
  repoParameters.insert("deviceFamily", deviceFamily());

  foreach (const QString &section, configSections){
    repoSettings->beginGroup(section);
    if (repoSettings->contains(repoName)){
      r = repoSettings->value(repoName).toString();
      repoSettings->endGroup();
      break;
    }
    repoSettings->endGroup();
  }

  QHashIterator<QString, QString> i(repoParameters);
  while (i.hasNext()){
    i.next();
    r.replace(
      QString("%(%1)").arg(i.key()),
      i.value());
  }

  return r;
}

void Ssu::requestFinished(QNetworkReply *reply){
  QSslConfiguration sslConfiguration = reply->sslConfiguration();

  qDebug() << sslConfiguration.peerCertificate().issuerInfo(QSslCertificate::CommonName);
  qDebug() << sslConfiguration.peerCertificate().subjectInfo(QSslCertificate::CommonName);

  foreach (const QSslCertificate cert, sslConfiguration.peerCertificateChain()){
    qDebug() << "Cert from chain" << cert.subjectInfo(QSslCertificate::CommonName);
  }

  if (reply->error() > 0){
    setError(reply->errorString());
    return;
  } else {
    QByteArray data = reply->readAll();
    qDebug() << "RequestOutput" << data;

    QDomDocument doc;
    QString xmlError;
    if (!doc.setContent(data, &xmlError)){
      setError(tr("Unable to parse server response (%1)").arg(xmlError));
      return;
    }

    QString action = doc.elementsByTagName("action").at(0).toElement().text();

    if (!verifyResponse(&doc)) return;

    if (action == "register"){
      if (!registerDevice(&doc)) return;
    } else if (action == "credentials"){
      if (!setCredentials(&doc)) return;
    } else {
      setError(tr("Response to unknown action encountered: %1").arg(action));
      return;
    }

    emit done();
  }
}

void Ssu::sendRegistration(QString username, QString password){
  errorFlag = false;

  QString ssuCaCertificate, ssuRegisterUrl;
  if (!settings->contains("ca-certificate")){
    setError("CA certificate for SSU not set (config key 'ca-certificate')");
    return;
  } else
    ssuCaCertificate = settings->value("ca-certificate").toString();

  if (!settings->contains("register-url")){
    setError("URL for SSU registration not set (config key 'register-url')");
    return;
  } else
    ssuRegisterUrl = settings->value("register-url").toString();

  QString IMEI = deviceUid();
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
  form.addQueryItem("deviceModel", deviceModel());

  qDebug() << "Sending request to " << request.url();
  QNetworkReply *reply;

  reply = manager->post(request, form.encodedQuery());
  // we could expose downloadProgress() from reply in case we want progress info
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
  emit done();
}

void Ssu::setFlavour(QString flavour){
  settings->setValue("flavour", flavour);
  emit flavourChanged();
}

void Ssu::setRelease(QString release, bool rnd){
  if (rnd)
    settings->setValue("rndRelease", release);
  else
    settings->setValue("release", release);
}

void Ssu::updateCredentials(bool force){
  errorFlag = false;

  if (deviceUid() == ""){
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
    setError("URL for credentials update not set (config key 'credentials-url')");
    return;
  } else
    ssuCredentialsUrl = settings->value("credentials-url").toString();

  if (!isRegistered()){
    setError("Device is not registered.");
    return;
  }

  if (!force){
    // skip updating if the last update was less than a day ago
    QDateTime now = QDateTime::currentDateTime();

    if (settings->contains("lastCredentialsUpdate")){
      QDateTime last = settings->value("lastCredentialsUpdate").toDateTime();
      if (last >= now.addDays(-1)){
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
  request.setUrl(QUrl(ssuCredentialsUrl.arg(deviceUid())));

  qDebug() << request.url();
  request.setSslConfiguration(sslConfiguration);

  QUrl form;
  form.addQueryItem("protocolVersion", SSU_PROTOCOL_VERSION);

  QNetworkReply *reply = manager->get(request);
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
