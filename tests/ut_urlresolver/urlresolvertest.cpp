/**
 * @file urlresolvertest.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2012
 */

#include "urlresolvertest.h"

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
