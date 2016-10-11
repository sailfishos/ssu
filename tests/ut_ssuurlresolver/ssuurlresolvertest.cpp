/**
 * @file ssuurlresolvertest.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#include "ssuurlresolvertest.h"

#include <stdlib.h>
#include <zypp/media/UrlResolverPlugin.h>

#include <QtTest/QtTest>

#include "libssu/sandbox_p.h"

/**
 * @class SsuUrlResolverTest
 * @brief Tests libzypp UrlResolverPlugin plugin compatibility
 *
 * This test verifies the UrlResolverPlugin works well with installed version of libzypp.
 */

void SsuUrlResolverTest::initTestCase()
{
    m_sandbox = new Sandbox(QString("%1/configroot").arg(LOCATE_DATA_PATH),
                            Sandbox::UseDirectly, Sandbox::ChildProcesses);
    if (!m_sandbox->activate()) {
        QFAIL("Failed to activate sandbox");
    }
    if (getenv("SSU_SANDBOX_PATH")) {
        qDebug() << "Using in-tree sandbox";
        setenv("LD_PRELOAD", getenv("SSU_SANDBOX_PATH"), 1);
    } else
        setenv("LD_PRELOAD", qPrintable(QString("%1/libsandboxhook.so").arg(TESTS_PATH)), 1);
}

void SsuUrlResolverTest::cleanupTestCase()
{
    delete m_sandbox;
    m_sandbox = 0;
}

void SsuUrlResolverTest::test_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("basic")
            << "plugin:ssu?repo=mer-core&debug&arch=i586"
            << "https://packages.testing.com//mer/i586/debug/";
}

void SsuUrlResolverTest::test()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);

    zypp::media::UrlResolverPlugin::HeaderList customHeaders;
    const QString resolved = QString::fromStdString(
                                 zypp::media::UrlResolverPlugin::resolveUrl(input.toStdString(), customHeaders).asString());

    QCOMPARE(resolved, expected);
}
