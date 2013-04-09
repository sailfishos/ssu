/**
 * @file process.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#include "process.h"

/**
 * @class Wraps QProcess for easier use within test code
 *
 * Example use:
 *
 * @code
 * Process ssu;
 * const QString output = ssu.execute("ssu", QStringList() << "mode");
 * QVERIFY2(!ssu.hasError(), qPrintable(ssu.fmtErrorMessage()));
 *
 * QCOMPARE(output, "...");
 * @endcode
 */

Process::Process() : m_expectFail(false), m_timedOut(false) {}

QString Process::execute(const QString &program, const QStringList &arguments,
    bool expectedResult){
  Q_ASSERT(m_process.state() == QProcess::NotRunning);
  m_program = program;
  m_arguments = arguments;
  m_expectFail = expectedResult == ExpectFail;
  m_process.start(program, arguments);
  m_timedOut = !m_process.waitForFinished();
  return m_process.readAllStandardOutput();
}

bool Process::hasError(){
  return m_timedOut
    || m_process.error() != QProcess::UnknownError
    || m_process.exitStatus() != QProcess::NormalExit
    || (m_process.exitCode() != 0) != m_expectFail;
}

QString Process::fmtErrorMessage(){
  if (!hasError()){
    return QString();
  }

  QStringList reasons;
  if (m_timedOut){
    reasons.append("Timed out");
  }else if (m_process.exitStatus() != QProcess::NormalExit){
    reasons.append("Process crashed");
  }else if (m_expectFail && (m_process.exitCode() == 0)){
    reasons.append("Did not fail while it was expected to");
  }else{
    if (m_process.error() != QProcess::UnknownError){
      reasons.append(m_process.errorString());
    }
    const QString errorOut = m_process.readAllStandardError();
    if (!errorOut.isEmpty()){
      reasons.append(errorOut);
    }
  }

  return QString("Failed to execute `%1 %2`: %3")
    .arg(m_program)
    .arg(QStringList(m_arguments).replaceInStrings(QRegExp("^|$"), "\"").join(" "))
    .arg(reasons.join(": "));
}
