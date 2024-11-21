/**
 * @file sandboxtest.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#include <QtTest/QtTest>
#include <QObject>

#include "libssu/sandbox_p.h"

class SandboxTest: public QObject
{
    Q_OBJECT

private slots:
    void test();

private:
    static QString readAll(const QString &fileName);
};

void SandboxTest::test()
{
    const QDir::Filters noHidden = QDir::AllEntries | QDir::NoDotAndDotDot;

    QCOMPARE(QDir(Sandbox::map(LOCATE_DATA_PATH, "/world")).entryList(noHidden, QDir::Name),
             QStringList()
             << "world-and-sandbox"
             << "world-only"
             << "world-only-to-be-copied-into-sandbox");

    QVERIFY(!QFileInfo(Sandbox::map(LOCATE_DATA_PATH, "/world/world-only")).isWritable());
    QCOMPARE(readAll(Sandbox::map(LOCATE_DATA_PATH, "/world/world-only")).trimmed(),
             QString("world/world-only"));

    QVERIFY(!QFileInfo(Sandbox::map(LOCATE_DATA_PATH, "/world/world-and-sandbox")).isWritable());
    QCOMPARE(readAll(Sandbox::map(LOCATE_DATA_PATH, "/world/world-and-sandbox")).trimmed(),
             QString("world/world-and-sandbox"));

    QVERIFY(!QFileInfo(Sandbox::map(LOCATE_DATA_PATH, "/world/world-only-to-be-copied-into-sandbox"))
            .isWritable());
    QCOMPARE(readAll(Sandbox::map(LOCATE_DATA_PATH, "/world/world-only-to-be-copied-into-sandbox"))
             .trimmed(), QString("world/world-only-to-be-copied-into-sandbox"));

    QVERIFY(!QFileInfo(Sandbox::map(LOCATE_DATA_PATH, "/world/sandbox-only")).exists());

    Sandbox sandbox(Sandbox::map(LOCATE_DATA_PATH, "/sandbox"),
                    Sandbox::UseAsSkeleton, Sandbox::ThisProcess | Sandbox::ChildProcesses);
    sandbox.addWorldFiles(Sandbox::map(LOCATE_DATA_PATH, "/world"), QDir::AllEntries,
                          QStringList() << "*-to-be-copied-into-sandbox");
    QVERIFY(sandbox.activate());

    QCOMPARE(QDir(Sandbox::map(LOCATE_DATA_PATH, "/world")).entryList(noHidden, QDir::Name),
             QStringList()
             << "sandbox-only"
             << "world-and-sandbox"
             << "world-only-to-be-copied-into-sandbox");

    QVERIFY(!QFileInfo(Sandbox::map(LOCATE_DATA_PATH, "/world/world-only")).exists());

    QVERIFY(QFileInfo(Sandbox::map(LOCATE_DATA_PATH, "/world/world-and-sandbox")).isWritable());
    QCOMPARE(readAll(Sandbox::map(LOCATE_DATA_PATH, "/world/world-and-sandbox")).trimmed(),
             QString("sandbox/world-and-sandbox"));

    QVERIFY(QFileInfo(Sandbox::map(LOCATE_DATA_PATH, "/world/world-only-to-be-copied-into-sandbox"))
            .isWritable());
    QCOMPARE(readAll(Sandbox::map(LOCATE_DATA_PATH, "/world/world-only-to-be-copied-into-sandbox"))
             .trimmed(), QString("world/world-only-to-be-copied-into-sandbox"));

    QVERIFY(QFileInfo(Sandbox::map(LOCATE_DATA_PATH, "/world/sandbox-only")).exists());
    QVERIFY(QFileInfo(Sandbox::map(LOCATE_DATA_PATH, "/world/sandbox-only")).isWritable());
    QCOMPARE(readAll(Sandbox::map(LOCATE_DATA_PATH, "/world/sandbox-only")).trimmed(),
             QString("sandbox/sandbox-only"));
}

QString SandboxTest::readAll(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("%s: Failed to open file for reading: '%s': %s", Q_FUNC_INFO, qPrintable(fileName),
                 qPrintable(file.errorString()));
        return QString();
    }

    return file.readAll();
}

QTEST_APPLESS_MAIN(SandboxTest)
#include "sandboxtest.moc"
