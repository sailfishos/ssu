/**
 * @file sandbox_p.h
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#ifndef _SANDBOX_P_H
#define _SANDBOX_P_H

#include <QtCore/QDir>
#include <QtCore/QString>

class Sandbox {
  class FileEngineHandler;

  public:
    enum Usage {
      UseDirectly,
      UseAsSkeleton,
    };

    enum Scope {
      ThisProcess    = 0x01,
      ChildProcesses = 0x02,
    };
    Q_DECLARE_FLAGS(Scopes, Scope)

  public:
    Sandbox();
    Sandbox(const QString &sandboxPath, Usage usage, Scopes scopes);
    ~Sandbox();

    bool isActive() const;

    void addWorldFiles(const QString &directory, QDir::Filters filters = QDir::NoFilter,
        const QStringList &filterNames = QStringList());

  private:
    bool copyFile(QAbstractFileEngine *src, QAbstractFileEngine *dst);

  private:
    static Sandbox *s_instance;
    QString m_sandboxPath;
    QString m_tempDir;
    FileEngineHandler *m_handler;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Sandbox::Scopes)

#endif
