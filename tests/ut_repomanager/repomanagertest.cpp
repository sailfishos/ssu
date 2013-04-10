/**
 * @file repomanagertest.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#include "repomanagertest.h"

#include <QtTest/QtTest>

#include "libssu/ssucoreconfig.h"
#include "libssu/ssurepomanager.h"

void RepoManagerTest::testSettings(){
  SsuCoreConfig *const coreConfig = SsuCoreConfig::instance();
  SsuRepoManager repoManager;

  repoManager.add("repo1", "http://repo1");
  QCOMPARE(coreConfig->value("repository-urls/repo1").toString(), QString("http://repo1"));
  QVERIFY(!coreConfig->value("enabled-repos").toStringList().contains("repo1"));
  QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));

  repoManager.enable("repo1");
  QCOMPARE(coreConfig->value("repository-urls/repo1").toString(), QString("http://repo1"));
  QVERIFY(!coreConfig->value("enabled-repos").toStringList().contains("repo1"));
  QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));

  repoManager.add("repo1");
  QCOMPARE(coreConfig->value("repository-urls/repo1").toString(), QString("http://repo1"));
  QVERIFY(coreConfig->value("enabled-repos").toStringList().contains("repo1"));
  QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));

  repoManager.disable("repo1");
  QCOMPARE(coreConfig->value("repository-urls/repo1").toString(), QString("http://repo1"));
  QVERIFY(coreConfig->value("enabled-repos").toStringList().contains("repo1"));
  QVERIFY(coreConfig->value("disabled-repos").toStringList().contains("repo1"));

  repoManager.enable("repo1");
  QCOMPARE(coreConfig->value("repository-urls/repo1").toString(), QString("http://repo1"));
  QVERIFY(coreConfig->value("enabled-repos").toStringList().contains("repo1"));
  QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));

  repoManager.add("repo2", "http://repo2");
  QCOMPARE(coreConfig->value("repository-urls/repo1").toString(), QString("http://repo1"));
  QVERIFY(coreConfig->value("enabled-repos").toStringList().contains("repo1"));
  QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));
  QCOMPARE(coreConfig->value("repository-urls/repo2").toString(), QString("http://repo2"));
  QVERIFY(!coreConfig->value("enabled-repos").toStringList().contains("repo2"));
  QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo2"));

  repoManager.disable("repo2");
  QCOMPARE(coreConfig->value("repository-urls/repo1").toString(), QString("http://repo1"));
  QVERIFY(coreConfig->value("enabled-repos").toStringList().contains("repo1"));
  QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));
  QCOMPARE(coreConfig->value("repository-urls/repo2").toString(), QString("http://repo2"));
  QVERIFY(!coreConfig->value("enabled-repos").toStringList().contains("repo2"));
  QVERIFY(coreConfig->value("disabled-repos").toStringList().contains("repo2"));

  repoManager.enable("repo2");
  QCOMPARE(coreConfig->value("repository-urls/repo1").toString(), QString("http://repo1"));
  QVERIFY(coreConfig->value("enabled-repos").toStringList().contains("repo1"));
  QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));
  QCOMPARE(coreConfig->value("repository-urls/repo2").toString(), QString("http://repo2"));
  QVERIFY(!coreConfig->value("enabled-repos").toStringList().contains("repo2"));
  QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo2"));

  repoManager.add("repo2");
  QCOMPARE(coreConfig->value("repository-urls/repo1").toString(), QString("http://repo1"));
  QVERIFY(coreConfig->value("enabled-repos").toStringList().contains("repo1"));
  QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));
  QCOMPARE(coreConfig->value("repository-urls/repo2").toString(), QString("http://repo2"));
  QVERIFY(coreConfig->value("enabled-repos").toStringList().contains("repo2"));
  QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo2"));

  repoManager.remove("repo1");
  QVERIFY(!coreConfig->contains("repository-urls/repo1"));
  QVERIFY(!coreConfig->value("enabled-repos").toStringList().contains("repo1"));
  QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));
  QCOMPARE(coreConfig->value("repository-urls/repo2").toString(), QString("http://repo2"));
  QVERIFY(coreConfig->value("enabled-repos").toStringList().contains("repo2"));
  QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo2"));

  repoManager.remove("repo2");
  QVERIFY(!coreConfig->contains("repository-urls/repo1"));
  QVERIFY(!coreConfig->value("enabled-repos").toStringList().contains("repo1"));
  QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));
  QVERIFY(!coreConfig->contains("repository-urls/repo2"));
  QVERIFY(!coreConfig->value("enabled-repos").toStringList().contains("repo2"));
  QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo2"));
}
