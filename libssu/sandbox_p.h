/**
 * @file sandbox_p.h
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#ifndef _SANDBOX_P_H
#define _SANDBOX_P_H

#include <QtCore/QSet>
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

  private:
    static Sandbox *s_instance;
    static QSet<QString> s_ssuConfigFiles;
    static QSet<QString> s_ssuConfigDirectories;
    QString m_sandboxPath;
    QString m_tempDir;
    FileEngineHandler *m_handler;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Sandbox::Scopes)

#endif
