/**
 * @file upgradetesthelper.h
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#ifndef _UPGRADETESTHELPER_H
#define _UPGRADETESTHELPER_H

#include <QtCore/QString>
#include <QtCore/QStringList>

class QIODevice;
class QSettings;
class QTextStream;

class UpgradeTestHelper {
  public:
    enum { HistoryLength = 5, CurrentVersion = 3 };

    struct TestCase;

    static QList<TestCase> readRecipe(QIODevice *recipe);
    static void fillSettings(QSettings *settings, const QList<TestCase> &testCases);
    static void fillDefaultSettings(QSettings *defaultSettings, const QList<TestCase> &testCases);
    static bool generateSnapshotRecipe(QTextStream *out);

    static QStringList groups();
};

struct UpgradeTestHelper::TestCase {
  TestCase(const QString &history, const QString &current, const QString &expected) :
    m_history(history), m_current(current), m_expected(expected){
  }

  QString history() const { return m_history; }
  QString current() const { return m_current; }
  QString expected() const { return m_expected; }

  QString key() const{
    return QString("%1__%2").arg(m_history).arg(m_current);
  }

  bool keyShouldBeSet() const{
    return m_expected != "@NOTSET@";
  }

  QString m_history; // Sequence of (S)et, (K)eep, (R)emove, (N)oop
  QString m_current;
  QString m_expected;
};

#endif
