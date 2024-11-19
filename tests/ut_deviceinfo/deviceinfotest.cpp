/**
 * @file deviceinfotest.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#include <QtTest/QtTest>
#include <QObject>

#include "libssu/ssudeviceinfo.h"
#include "libssu/sandbox_p.h"

class DeviceInfoTest: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testAdaptationVariables();
    void testFeatureVariables();
    void testDeviceUid();
    void testVariableSection();
    void testValue();

private:
    Sandbox *m_sandbox = nullptr;
};

void DeviceInfoTest::initTestCase()
{
    m_sandbox = new Sandbox(QString("%1/configroot").arg(LOCATE_DATA_PATH),
                            Sandbox::UseAsSkeleton, Sandbox::ThisProcess);
    if (!m_sandbox->activate()) {
        qFatal("Failed to activate sandbox");
    }
}

void DeviceInfoTest::cleanupTestCase()
{
    delete m_sandbox;
    m_sandbox = nullptr;
}

void DeviceInfoTest::testAdaptationVariables()
{
    SsuDeviceInfo deviceInfo("N950");
    QHash<QString, QString> variables;
    QHash<QString, QString> variablesExpected;
    variablesExpected["adaptation"] = "n950-n9";
    variablesExpected["foo-n9"] = "foo-n9-val";
    variablesExpected["foo-n950-n9"] = "foo-n950-n9-val";

    QString repoName = deviceInfo.adaptationVariables("adaptation1", &variables);

    QCOMPARE(variables, variablesExpected);
    QCOMPARE(repoName, QString("adaptation"));
}

void DeviceInfoTest::testFeatureVariables()
{
    SsuDeviceInfo deviceInfo("N950");
    QHash<QString, QString> featureVariables;
    QString repoName = deviceInfo.adaptationVariables("feature1", &featureVariables);
    QHash<QString, QString> featureVariablesExpected;
    featureVariablesExpected["feature"] = "test";
    QCOMPARE(featureVariables, featureVariablesExpected);
    QCOMPARE(repoName, QString("feature1"));
}

void DeviceInfoTest::testDeviceUid()
{
    QVERIFY2(!SsuDeviceInfo().deviceUid().isEmpty(), "No method to get device UID on this platform");
}

void DeviceInfoTest::testVariableSection()
{
    SsuDeviceInfo deviceInfo;

    QHash<QString, QString> fooVars;
    QHash<QString, QString> fooVarsExpected;
    fooVarsExpected["foo1"] = "foo1Val";
    fooVarsExpected["foo2"] = "foo2Val";

    deviceInfo.variableSection("foo", &fooVars);

    QCOMPARE(fooVars, fooVarsExpected);

    QHash<QString, QString> barVars;
    QHash<QString, QString> barVarsExpected;
    barVarsExpected["bar1"] = "bar1Val";
    barVarsExpected["bar2"] = "bar2Val";

    deviceInfo.variableSection("bar", &barVars);

    QCOMPARE(barVars, barVarsExpected);

    QHash<QString, QString> bazVars;
    QHash<QString, QString> bazVarsExpected;
    bazVarsExpected["foo1"] = "foo1Val";
    bazVarsExpected["foo2"] = "foo2Val";
    bazVarsExpected["bar1"] = "bar1Val";
    bazVarsExpected["bar2"] = "bar2Val";

    deviceInfo.variableSection("baz", &bazVars);

    QCOMPARE(bazVars, bazVarsExpected);
}

void DeviceInfoTest::testValue()
{
    SsuDeviceInfo deviceInfo("N950");
    QCOMPARE(deviceInfo.value("family").toString(), QString("n950-n9"));
    QCOMPARE(deviceInfo.value("adaptation-repos").toStringList(),
             QString("n9xx-common,n950-n9").split(','));
    QCOMPARE(deviceInfo.value("foo").toString(), QString("n950-foo"));
}

QTEST_APPLESS_MAIN(DeviceInfoTest)
#include "deviceinfotest.moc"
