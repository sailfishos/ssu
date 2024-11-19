/**
 * @file repomanagertest.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 * @todo simulate device specific repos via boardmappings
 * @todo test repository filtering
 */

#include <QtTest/QtTest>
#include <QObject>
#include <QStringList>

#include "libssu/ssucoreconfig_p.h"
#include "libssu/ssurepomanager.h"
#include "libssu/sandbox_p.h"

class RepoManagerTest: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testSettings();
    void testCustomRepos();
    void testRepos();

private:
    QStringList rndRepos, releaseRepos;
    Sandbox *m_sandbox;
};

void RepoManagerTest::initTestCase()
{
    m_sandbox = new Sandbox(QString("%1/configroot").arg(LOCATE_DATA_PATH),
                            Sandbox::UseAsSkeleton, Sandbox::ThisProcess);
    if (!m_sandbox->activate()) {
        qFatal("Failed to activate sandbox");
    }

    rndRepos << "mer-core" << "adaptation" << "nemo" << "non-oss" << "oss";
    releaseRepos << "vendor" << "apps";
}

void RepoManagerTest::cleanupTestCase()
{
    delete m_sandbox;
    m_sandbox = nullptr;
}

void RepoManagerTest::testSettings()
{
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
    QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));

    repoManager.disable("repo1");
    QCOMPARE(coreConfig->value("repository-urls/repo1").toString(), QString("http://repo1"));
    QVERIFY(coreConfig->value("disabled-repos").toStringList().contains("repo1"));

    repoManager.enable("repo1");
    QCOMPARE(coreConfig->value("repository-urls/repo1").toString(), QString("http://repo1"));
    QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));

    repoManager.add("repo2", "http://repo2");
    QCOMPARE(coreConfig->value("repository-urls/repo1").toString(), QString("http://repo1"));
    QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));
    QCOMPARE(coreConfig->value("repository-urls/repo2").toString(), QString("http://repo2"));
    QVERIFY(!coreConfig->value("enabled-repos").toStringList().contains("repo2"));
    QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo2"));

    repoManager.disable("repo2");
    QCOMPARE(coreConfig->value("repository-urls/repo1").toString(), QString("http://repo1"));
    QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));
    QCOMPARE(coreConfig->value("repository-urls/repo2").toString(), QString("http://repo2"));
    QVERIFY(!coreConfig->value("enabled-repos").toStringList().contains("repo2"));
    QVERIFY(coreConfig->value("disabled-repos").toStringList().contains("repo2"));

    repoManager.enable("repo2");
    QCOMPARE(coreConfig->value("repository-urls/repo1").toString(), QString("http://repo1"));
    QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));
    QCOMPARE(coreConfig->value("repository-urls/repo2").toString(), QString("http://repo2"));
    QVERIFY(!coreConfig->value("enabled-repos").toStringList().contains("repo2"));
    QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo2"));

    repoManager.add("repo2");
    QCOMPARE(coreConfig->value("repository-urls/repo1").toString(), QString("http://repo1"));
    QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));
    QCOMPARE(coreConfig->value("repository-urls/repo2").toString(), QString("http://repo2"));
    QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo2"));

    repoManager.remove("repo1");
    QVERIFY(!coreConfig->contains("repository-urls/repo1"));
    QVERIFY(!coreConfig->value("enabled-repos").toStringList().contains("repo1"));
    QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));
    QCOMPARE(coreConfig->value("repository-urls/repo2").toString(), QString("http://repo2"));
    QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo2"));

    repoManager.remove("repo2");
    QVERIFY(!coreConfig->contains("repository-urls/repo1"));
    QVERIFY(!coreConfig->value("enabled-repos").toStringList().contains("repo1"));
    QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo1"));
    QVERIFY(!coreConfig->contains("repository-urls/repo2"));
    QVERIFY(!coreConfig->value("enabled-repos").toStringList().contains("repo2"));
    QVERIFY(!coreConfig->value("disabled-repos").toStringList().contains("repo2"));
}

void RepoManagerTest::testCustomRepos()
{
    SsuCoreConfig *const coreConfig = SsuCoreConfig::instance();
    SsuRepoManager repoManager;
    QStringList customRepos;
    QSet<QString> set;

    repoManager.add("r1", "http://localhost/r1/");
    repoManager.add("r2", "http://localhost/r2/");
    customRepos << "r1" << "r2";

    QCOMPARE(releaseRepos.toSet().count(), 2);

    // check default release repos + custom repos
    coreConfig->setDeviceMode(Ssu::ReleaseMode, Ssu::Replace);
    QCOMPARE(coreConfig->deviceMode(), Ssu::ReleaseMode);
    set = releaseRepos.toSet().unite(repoManager.repos().toSet());
    QCOMPARE(set.count(), 4);
    QVERIFY(set == customRepos.toSet().unite(releaseRepos.toSet()));

    // check default rnd repos + custom repos
    coreConfig->setDeviceMode(Ssu::RndMode, Ssu::Replace);
    QCOMPARE(coreConfig->deviceMode(), Ssu::RndMode);
    set = rndRepos.toSet().unite(repoManager.repos().toSet());
    QCOMPARE(set.count(), 7);
    QVERIFY(set == customRepos.toSet().unite(rndRepos.toSet()));

    // enable store repository as preconfigured custom repo
    customRepos << "store";
    repoManager.add("store");
    set = rndRepos.toSet().unite(repoManager.repos().toSet());
    QCOMPARE(set.count(), 8);
    QVERIFY(set == customRepos.toSet().unite(rndRepos.toSet()));

    // check store repo with release mode
    coreConfig->setDeviceMode(Ssu::ReleaseMode, Ssu::Replace);
    QCOMPARE(coreConfig->deviceMode(), Ssu::ReleaseMode);
    set = releaseRepos.toSet().unite(repoManager.repos().toSet());
    QCOMPARE(set.count(), 5);
    QVERIFY(set == customRepos.toSet().unite(releaseRepos.toSet()));

    // check repositories in update mode
    // store + other custom repositories should be gone
    coreConfig->setDeviceMode(Ssu::UpdateMode, Ssu::Add);
    QCOMPARE(coreConfig->deviceMode(), Ssu::ReleaseMode | Ssu::UpdateMode);
    QVERIFY(releaseRepos.toSet() == repoManager.repos().toSet());

    // check rndmode + update mode
    coreConfig->setDeviceMode(Ssu::RndMode, Ssu::Add);
    QCOMPARE(coreConfig->deviceMode(), Ssu::ReleaseMode | Ssu::UpdateMode | Ssu::RndMode);
    QVERIFY(rndRepos.toSet() == repoManager.repos().toSet());

    coreConfig->setDeviceMode(Ssu::RndMode | Ssu::UpdateMode, Ssu::Replace);
    QCOMPARE(coreConfig->deviceMode(), Ssu::UpdateMode | Ssu::RndMode);
    QVERIFY(rndRepos.toSet() == repoManager.repos().toSet());

    // check appinstall mode
    // custom repositories, apart from store, should be gone
    customRepos.clear();
    customRepos << "store";

    // check release mode
    coreConfig->setDeviceMode(Ssu::ReleaseMode | Ssu::AppInstallMode, Ssu::Replace);
    QCOMPARE(coreConfig->deviceMode(), Ssu::ReleaseMode | Ssu::AppInstallMode);
    set = releaseRepos.toSet().unite(repoManager.repos().toSet());
    QCOMPARE(set.count(), 3);
    QVERIFY(set == customRepos.toSet().unite(releaseRepos.toSet()));

    // and rnd mode
    coreConfig->setDeviceMode(Ssu::RndMode, Ssu::Add);
    QCOMPARE(coreConfig->deviceMode(), Ssu::ReleaseMode | Ssu::AppInstallMode | Ssu::RndMode);
    set = rndRepos.toSet().unite(repoManager.repos().toSet());
    QCOMPARE(set.count(), 6);
    QVERIFY(set == customRepos.toSet().unite(rndRepos.toSet()));

    coreConfig->setDeviceMode(Ssu::RndMode | Ssu::AppInstallMode, Ssu::Replace);
    QCOMPARE(coreConfig->deviceMode(), Ssu::AppInstallMode | Ssu::RndMode);
    set = rndRepos.toSet().unite(repoManager.repos().toSet());
    QCOMPARE(set.count(), 6);
    QVERIFY(set == customRepos.toSet().unite(rndRepos.toSet()));
}

void RepoManagerTest::testRepos()
{
    SsuCoreConfig *const coreConfig = SsuCoreConfig::instance();
    SsuRepoManager repoManager;
    QSet<QString> set;

    // first, check if the repository lists are correct by explicitely
    // specifying rnd and release modes
    set = rndRepos.toSet().subtract(repoManager.repos(true).toSet());
    QVERIFY(set.isEmpty());

    set = releaseRepos.toSet().subtract(repoManager.repos(false).toSet());
    QVERIFY(set.isEmpty());

    // second, check if changing the device mode in the configuration
    // properly switches between modes
    coreConfig->setDeviceMode(Ssu::ReleaseMode, Ssu::Replace);
    QCOMPARE(coreConfig->deviceMode(), Ssu::ReleaseMode);
    set = releaseRepos.toSet().subtract(repoManager.repos().toSet());
    QVERIFY(set.isEmpty());

    // both adding rnd-mode on top of release mode, and setting it
    // to rnd mode only should switch the device to rnd mode
    coreConfig->setDeviceMode(Ssu::RndMode, Ssu::Add);
    QCOMPARE(coreConfig->deviceMode(), Ssu::ReleaseMode | Ssu::RndMode);
    set = rndRepos.toSet().subtract(repoManager.repos().toSet());
    QVERIFY(set.isEmpty());

    coreConfig->setDeviceMode(Ssu::RndMode, Ssu::Replace);
    QCOMPARE(coreConfig->deviceMode(), Ssu::RndMode);
    set = rndRepos.toSet().subtract(repoManager.repos().toSet());
    QVERIFY(set.isEmpty());
}

QTEST_APPLESS_MAIN(RepoManagerTest)
#include "repomanagertest.moc"
