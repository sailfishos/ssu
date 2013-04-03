/**
 * @file process.h
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#ifndef _PROCESS_H
#define _PROCESS_H

#include <QtCore/QProcess>

class Process {
  public:
    enum ExpectedResult {
      ExpectSuccess,
      ExpectFail
    };

  public:
    Process();

    QString execute(const QString &program, const QStringList &arguments,
        bool expectedResult = ExpectSuccess);
    bool hasError();
    QString fmtErrorMessage();

  private:
    QProcess m_process;
    QString m_program;
    QStringList m_arguments;
    bool m_expectFail;
    bool m_timedOut;
};

#endif
