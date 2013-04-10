/**
 * @file upgradetesthelper.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#include "upgradetesthelper.h"

#include <QtCore/QBuffer>
#include <QtCore/QDebug>
#include <QtCore/qmath.h>
#include <QtCore/QTemporaryFile>
#include <QtCore/QTextStream>

#include "libssu/ssusettings.h"

/**
 * @class UpgradeTestHelper
 * @brief Utilities to generate upgrade-test data
 *
 * It is driven by recipe in following format.
 *
 * Every line consists of three values delimited by colon:
 *
 * @verbatim
 * <history>:<current>:<expected>
 * @endverbatim
 *
 * - history of changes of the key. A word of length HistoryLength made up
 *   of letters [SKRN] -- (S)et value, (K)eep value, (R)emove key, (N)oop
 * - current value (settings at revision CurrentVersion)
 * - value expected after upgrade. The special value "@NOTSET@" means the key
 *   is expected to not be set after upgrade.
 *
 * Action (S)et results in assigning "v<revision>-default" to the key, e.g.,
 * "v5-default" when 'S' appears as 5th letter in the history word.
 *
 * The key is built as "<history>__<current>".
 *
 * Example:
 *
 * @verbatim
 * SKNRN:v1-default:@NOTSET@
 * SSNSN:v2-default:v4-default
 * @endverbatim
 *
 * Generated settings:
 *
 * @verbatim
 * [Global]
 * configVersion = 2
 * SKNRN__v1-default = v1-default
 * SSNSN__v2-default = v2-default
 *
 * [groupA]
 * SKNRN__v1-default = v1-default
 * SSNSN__v2-default = v2-default
 * @endverbatim
 *
 * Generated default settings:
 *
 * @verbatim
 * [Global]
 * configVersion = 5
 *
 * [1]
 * SKNRN__v1-default = v1-default
 * groupA\SKNRN__v1-default = v1-default
 * SSNSN__v2-default = v1-default
 * groupA\SSNSN__v2-default = v1-default
 *
 * [2]
 * SKNRN__v1-default = v1-default
 * groupA\SKNRN__v1-default = v1-default
 * SSNSN__v2-default = v2-default
 * groupA\SSNSN__v2-default = v2-default
 *
 * [3]
 *
 * [4]
 * cmd-remove = SKNRN__v1-default, groupA/SKNRN__v1-default
 * SSNSN__v2-default = v4-default
 * groupA\SSNSN__v2-default = v4-default
 *
 * [5]
 * @endverbatim
 */

QList<UpgradeTestHelper::TestCase> UpgradeTestHelper::readRecipe(QIODevice *recipe){
  QList<TestCase> testCases;

  while (!recipe->atEnd()){
    const QString line = recipe->readLine().trimmed();
    if (line.startsWith('#') || line.isEmpty())
      continue;
    const QStringList splitted = line.split(':');
    Q_ASSERT_X(splitted.count() == 3, Q_FUNC_INFO,
        qPrintable(QString("Inalid recipe line '%1'").arg(line)));
    testCases.append(TestCase(splitted.at(0), splitted.at(1), splitted.at(2)));
  }

  return testCases;
}

void UpgradeTestHelper::fillSettings(QSettings *settings, const QList<TestCase> &testCases){
  settings->setValue("configVersion", CurrentVersion);

  foreach (const QString &group, groups()){
    settings->beginGroup(group);

    foreach (const TestCase &testCase, testCases){
      if (!testCase.current().isEmpty()){
        settings->setValue(testCase.key(), testCase.current());
      }
    }

    settings->endGroup();
  }

  settings->sync();
}

void UpgradeTestHelper::fillDefaultSettings(QSettings *defaultSettings, const QList<TestCase>
    &testCases){
  defaultSettings->setValue("configVersion", HistoryLength);

  QHash<QString, QString> lastSetValue; // for the (K)eep action; no need to qualify with group

  for (int revision = 1; revision <= HistoryLength; ++revision){
    defaultSettings->beginGroup(QString::number(revision));

    QStringList keysToRemove;

    foreach (const QString &group, groups()){
      defaultSettings->beginGroup(group);

      foreach (const TestCase &testCase, testCases){
        switch (testCase.history().at(revision - 1).toAscii()){
        case 'S': // (S)et value
          lastSetValue[testCase.key()] = QString("v%1-default").arg(revision);
          defaultSettings->setValue(testCase.key(), lastSetValue[testCase.key()]);
          break;

        case 'K': // (K)eep value
          Q_ASSERT_X(!lastSetValue[testCase.key()].isEmpty(), Q_FUNC_INFO,
              qPrintable(QString("Inalid TestCase::history: '%1'").arg(testCase.history())));
          defaultSettings->setValue(testCase.key(), lastSetValue[testCase.key()]);
          break;

        case 'R': // (R)emove key
          keysToRemove.append((group.isEmpty() ? group : group + "/") + testCase.key());
          lastSetValue.remove(testCase.key());
          break;

        case 'N': // (N)oop
          break;

        default:
          Q_ASSERT_X(false, Q_FUNC_INFO, qPrintable(QString(
                  "Inalid TestCase::history: '%1': invalid command-code '%2'")
                .arg(testCase.history())
                .arg(testCase.history().at(revision - 1))));
        }

      }

      defaultSettings->endGroup();
    }

    if (!keysToRemove.isEmpty()){
      defaultSettings->setValue("cmd-remove", keysToRemove);
    }

    defaultSettings->endGroup();
  }

  defaultSettings->sync();
}

bool UpgradeTestHelper::generateSnapshotRecipe(QTextStream *out){
  const QString actions = "SKRN";

  QBuffer buf;
  buf.open(QIODevice::ReadWrite);
  QTextStream stream(&buf);

  // for all "valid" variations of the letters "SKRN" of length HistoryLength
  for (int i = 0; i < qPow(actions.count(), HistoryLength); ++i){
    QString history = QString::number(i, actions.count());
    // Left pad to HistoryLength
    history.prepend(QString(HistoryLength - history.length(), '0'));
    for (int revision = 0; revision < history.length(); ++revision){
      history.replace(revision, 1, actions.at(history.at(revision).digitValue()));
    }

    static const QRegExp invalidSequence("(^[^S]*K|R[^S]*K|^R)");
    if (history.contains(invalidSequence)){
      continue;
    }

    for (int revision = 0; revision < HistoryLength; ++revision){
      stream << history << QString(":v%1-default:\n").arg(revision);
    }

    stream << history << ":custom:\n";
  }

  stream.flush();

  // Read recipe
  buf.seek(0);
  QList<TestCase> testCases = readRecipe(&buf);

  // Generate settings file according to recipe
  QTemporaryFile settingsFile;
  if (!settingsFile.open()){
    return false;
  }

  QSettings settings(settingsFile.fileName(), QSettings::IniFormat);

  fillSettings(&settings, testCases);

  // Generate defaults file according to recipe
  QTemporaryFile defaultSettingsFile;
  if (!defaultSettingsFile.open()){
    return false;
  }

  QSettings defaultSettings(defaultSettingsFile.fileName(), QSettings::IniFormat);

  fillDefaultSettings(&defaultSettings, testCases);

  // Parse settings -- do upgrade
  SsuSettings ssuSettings(settingsFile.fileName(), QSettings::IniFormat,
      defaultSettingsFile.fileName());

  // Output recipe
  foreach (const UpgradeTestHelper::TestCase &testCase, testCases){
    const QString expected = ssuSettings.contains(testCase.key())
      ? ssuSettings.value(testCase.key()).toString()
      : "@NOTSET@";
    *out << QString("%1:%2:%3\n")
      .arg(testCase.history())
      .arg(testCase.current())
      .arg(expected);
  }

  return true;
}

QStringList UpgradeTestHelper::groups(){
  static const QStringList groups = QStringList() << "" /* General */ << "groupA";
  return groups;
}
