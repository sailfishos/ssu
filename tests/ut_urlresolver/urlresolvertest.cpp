/**
 * @file urlresolvertest.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2012
 */

#include "urlresolvertest.h"
#include "constants.h"
#include "testutils/process.h"

void UrlResolverTest::initTestCase(){
#ifdef TARGET_ARCH
  // test will fail if executed without proper installation of libssu and repos
  QString arch = "";
  arch = TARGET_ARCH;
  rndRepos["nemo"] = QString("https://packages.example.com/nemo/latest/platform/%1/").arg(arch);
  rndRepos["mer-core"] = QString("https://packages.example.com/mer/latest/builds/%1/packages/").arg(arch);
/*
  rndRepos["non-oss"] = "";
*/
  releaseRepos["nemo"] = QString("https://packages.example.com/releases/0.1/nemo/platform/%1/").arg(arch);
  releaseRepos["mer-core"] = QString("https://packages.example.com/0.1/mer/%1/packages/").arg(arch);
  releaseRepos["jolla"] = QString("https://packages.example.com/releases/0.1/jolla/%1/").arg(arch);
#else
#warning "TARGET_ARCH not defined"
// if run in build dir, ssu.ini and repos missing the repo urls to compare will be empty!
  rndRepos["nemo"] = "";
  rndRepos["mer-core"] = "";
  /*
  rndRepos["non-oss"] = "";
  */
  releaseRepos["nemo"] = "";
  releaseRepos["mer-core"] = "";
  releaseRepos["jolla"] = "";
#endif
}

void UrlResolverTest::cleanupTestCase(){

}

void UrlResolverTest::checkFlavour(){
  ssu.setFlavour("testing");
  QCOMPARE(ssu.flavour(), QString("testing"));
  ssu.setFlavour("release");
  QCOMPARE(ssu.flavour(), QString("release"));
  Ssu ssu2;
  QCOMPARE(ssu2.flavour(), QString("release"));

}

void UrlResolverTest::checkRelease(){
  ssu.setRelease("0.1");
  QCOMPARE(ssu.release(), QString("0.1"));
  ssu.setRelease("0.2", true);
  QCOMPARE(ssu.release(), QString("0.1"));
  QCOMPARE(ssu.release(true), QString("0.2"));
  Ssu ssu2;
  QCOMPARE(ssu2.release(), QString("0.1"));
  QCOMPARE(ssu2.release(true), QString("0.2"));
  ssu.setRelease("latest", true);
}

void UrlResolverTest::checkDomain(){
  QString credentialsUrl;
  QString registerUrl;

  // domain not defined -> default
  ssu.setDomain("");
  credentialsUrl = ssu.repoUrl("credentials-url");
  QCOMPARE(credentialsUrl, QString("https://ssu.testing.com/ssu/device/%1/credentials.xml"));

  // domain defined but not matching block -> default
  ssu.setDomain("nevermind");
  credentialsUrl = ssu.repoUrl("credentials-url");
  QCOMPARE(credentialsUrl, QString("https://ssu.testing.com/ssu/device/%1/credentials.xml"));

  ssu.setDomain("example");
  QCOMPARE(ssu.domain(), QString("example"));
  credentialsUrl = ssu.repoUrl("credentials-url");
  QCOMPARE(credentialsUrl, QString("https://ssu.example.com/ssu/device/%1/credentials.xml"));
  registerUrl = ssu.repoUrl("register-url");
  QCOMPARE(registerUrl, QString("https://ssu.example.com/ssu/device/%1/register.xml"));

}
void UrlResolverTest::checkCleanUrl(){
  QHashIterator<QString, QString> i(rndRepos);
  while (i.hasNext()){
    i.next();
    QString url=ssu.repoUrl(i.key(), true);
    QVERIFY(!url.contains("%("));
  }

}

void UrlResolverTest::simpleRepoUrlLookup(){
  QHashIterator<QString, QString> i(rndRepos);
  while (i.hasNext()){
    QString url;
    i.next();
    url=ssu.repoUrl(i.key(), true);
    QCOMPARE(url, i.value());
    url=ssu.repoUrl(i.key(), false);
#ifdef TARGET_ARCH
    QVERIFY(url.compare(i.value()) != 0);
#else
    QCOMPARE(url, i.value());
#endif
  }
/*
  QCOMPARE(ssu.repoUrl("jolla", 0), QString(""));
  QCOMPARE(ssu.repoUrl("non-oss", 1), QString(""));
*/
}

void UrlResolverTest::checkReleaseRepoUrls(){
  QHashIterator<QString, QString> i(releaseRepos);
  while (i.hasNext()){
    QString url;
    i.next();
    url=ssu.repoUrl(i.key(), false);
    QCOMPARE(url, i.value());
    url=ssu.repoUrl(i.key());
    QCOMPARE(url, i.value());
  }
}

void UrlResolverTest::checkRegisterDevice(){
  QDomDocument doc("foo");

  QDomElement root = doc.createElement("foo");
  doc.appendChild(root);

  QDomElement certificate = doc.createElement("certificate");
  root.appendChild(certificate);

  QVERIFY2(!ssu.registerDevice(&doc),
      "Ssu::registerDevice() should fail when 'certificate' is empty");

  QFile certificateFile(TESTS_DATA_PATH "/mycert.crt");
  QVERIFY(certificateFile.open(QIODevice::ReadOnly));

  certificate.appendChild(doc.createTextNode(certificateFile.readAll()));

  QDomElement privateKey = doc.createElement("privateKey");
  root.appendChild(privateKey);

  QVERIFY2(!ssu.registerDevice(&doc),
      "Ssu::registerDevice() should fail when 'privateKey' is empty");

  QFile privateKeyFile(TESTS_DATA_PATH "/mykey.key");
  QVERIFY(privateKeyFile.open(QIODevice::ReadOnly));

  privateKey.appendChild(doc.createTextNode(privateKeyFile.readAll()));

  QDomElement user = doc.createElement("user");
  root.appendChild(user);
  user.appendChild(doc.createTextNode("john.doe"));

  QSignalSpy registrationStatusChanged_spy(&ssu, SIGNAL(registrationStatusChanged()));

  QVERIFY(ssu.registerDevice(&doc));

  QVERIFY(registrationStatusChanged_spy.count() == 1);
  QVERIFY(ssu.isRegistered());

  ssu.unregister();

  QVERIFY(registrationStatusChanged_spy.count() == 2);
  QVERIFY(!ssu.isRegistered());
}

void UrlResolverTest::checkSetCredentials(){
  QDomDocument doc("foo");

  QDomElement root = doc.createElement("foo");
  doc.appendChild(root);

  QDomElement credentials1 = doc.createElement("credentials");
  root.appendChild(credentials1);

  QVERIFY2(!ssu.setCredentials(&doc),
      "Ssu::setCredentials() should fail when 'scope' is not defined");

  credentials1.setAttribute("scope", "utscope1");

  QVERIFY2(!ssu.setCredentials(&doc),
      "Ssu::setCredentials() should fail when username/password is missing");

  QDomElement username1 = doc.createElement("username");
  credentials1.appendChild(username1);
  username1.appendChild(doc.createTextNode("john.doe1"));

  QVERIFY2(!ssu.setCredentials(&doc),
      "Ssu::setCredentials() should fail when password is missing");

  QDomElement password1 = doc.createElement("password");
  credentials1.appendChild(password1);
  password1.appendChild(doc.createTextNode("SeCrEt1"));

  QVERIFY2(ssu.setCredentials(&doc),
      qPrintable(QString("setCredentials() failed: %1").arg(ssu.lastError())));

  QVERIFY2(ssu.lastCredentialsUpdate() > QDateTime::currentDateTime().addSecs(-5) &&
      ssu.lastCredentialsUpdate() <= QDateTime::currentDateTime(),
      "Ssu::lastCredentialsUpdate was not updated");

  //QVERIFY(ssu.credentialScopes().contains("utscope1"));
  QCOMPARE(ssu.credentials("utscope1").first, QString("john.doe1"));
  QCOMPARE(ssu.credentials("utscope1").second, QString("SeCrEt1"));


  QDomElement credentials2 = doc.createElement("credentials");
  root.appendChild(credentials2);
  credentials2.setAttribute("scope", "utscope2");

  QDomElement username2 = doc.createElement("username");
  credentials2.appendChild(username2);
  username2.appendChild(doc.createTextNode("john.doe2"));

  QDomElement password2 = doc.createElement("password");
  credentials2.appendChild(password2);
  password2.appendChild(doc.createTextNode("SeCrEt2"));

  QVERIFY2(ssu.setCredentials(&doc),
      qPrintable(QString("setCredentials() failed: %1").arg(ssu.lastError())));

  QVERIFY2(ssu.lastCredentialsUpdate() > QDateTime::currentDateTime().addSecs(-5) &&
      ssu.lastCredentialsUpdate() <= QDateTime::currentDateTime(),
      "Ssu::lastCredentialsUpdate was not updated");

  //QVERIFY(ssu.credentialScopes().contains("utscope1"));
  QCOMPARE(ssu.credentials("utscope1").first, QString("john.doe1"));
  QCOMPARE(ssu.credentials("utscope1").second, QString("SeCrEt1"));

  //QVERIFY(ssu.credentialScopes().contains("utscope2"));
  QCOMPARE(ssu.credentials("utscope2").first, QString("john.doe2"));
  QCOMPARE(ssu.credentials("utscope2").second, QString("SeCrEt2"));
}

void UrlResolverTest::checkStoreAuthorizedKeys(){
  struct Cleanup {
    ~Cleanup(){
      if (!tempHomePath.isEmpty()){
        Process rmtemp;
        rmtemp.execute("rm", QStringList() << "-rf" << tempHomePath);
        if (rmtemp.hasError()){
          qWarning("%s: Failed to remove temporary directory '%s': %s", Q_FUNC_INFO,
              tempHomePath.constData(), qPrintable(rmtemp.fmtErrorMessage()));
        }

        if (!qputenv("HOME", originalHomePath)){
          qFatal("%s: Failed to restore HOME environment variable", Q_FUNC_INFO);
        }
      }
    }

    QByteArray originalHomePath;
    QByteArray tempHomePath;
  } cleanup;

  // Temporarily change HOME path so Ssu::storeAuthorizedKeys() does not touch
  // real home directory
  cleanup.originalHomePath = qgetenv("HOME");
  QVERIFY(!cleanup.originalHomePath.isEmpty());

  Process mktemp;
  cleanup.tempHomePath = mktemp.execute("mktemp",
      QStringList() << "-t" << "-d" << "ut_urlresolver.temp-home.XXX").trimmed().toLocal8Bit();
  QVERIFY2(!mktemp.hasError(), qPrintable(mktemp.fmtErrorMessage()));

  QVERIFY(qputenv("HOME", cleanup.tempHomePath));
  QVERIFY2(QDir::homePath() == QString(cleanup.tempHomePath),
      "QDir::homePath() does not change after qputenv(\"HOME\", \"...\")");

  // Here starts the test itself
  QByteArray testData("# test data\n");
  ssu.storeAuthorizedKeys(testData);

  QFile authorizedKeys(QDir::home().filePath(".ssh/authorized_keys"));
  QVERIFY(authorizedKeys.open(QIODevice::ReadOnly));

  QVERIFY(authorizedKeys.readAll().split('\n').contains(testData.trimmed()));

  QByteArray testData2("# test data2\n");
  ssu.storeAuthorizedKeys(testData2);

  QEXPECT_FAIL("", "Ssu::storeAuthorizedKeys() does not modify existing authorized_keys", Continue);
  authorizedKeys.seek(0);
  QVERIFY(authorizedKeys.readAll().split('\n').contains(testData2.trimmed()));

  const QFile::Permissions go_rwx =
    QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
    QFile::ReadOther | QFile::WriteOther | QFile::ExeOther;
  QVERIFY((QFileInfo(QDir::home().filePath(".ssh")).permissions() & go_rwx) == 0);
}

void UrlResolverTest::checkVerifyResponse(){
  QDomDocument doc("foo");

  QDomElement root = doc.createElement("foo");
  doc.appendChild(root);

  QDomElement action = doc.createElement("action");
  root.appendChild(action);
  action.appendChild(doc.createTextNode("register"));

  QDomElement deviceId = doc.createElement("deviceId");
  root.appendChild(deviceId);
  deviceId.appendChild(doc.createTextNode("deadbeef-dead-beef-dead"));

  QDomElement protocolVersion = doc.createElement("protocolVersion");
  root.appendChild(protocolVersion);

  QDomText protocolVersionText = doc.createTextNode(SSU_PROTOCOL_VERSION);
  protocolVersion.appendChild(protocolVersionText);

  QVERIFY(ssu.verifyResponse(&doc));

  protocolVersionText.setData(SSU_PROTOCOL_VERSION ".invalid");

  QVERIFY2(!ssu.verifyResponse(&doc),
      "Ssu::verifyResponse() should fail when protocolVersion does not match SSU_PROTOCOL_VERSION");
}
