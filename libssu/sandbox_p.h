/**
 * @file sandbox_p.h
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#ifndef _SANDBOX_P_H
#define _SANDBOX_P_H

#include <QtCore/QDir>
#include <QtCore/QSet>
#include <QtCore/QString>

class Sandbox {
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

    bool activate();
    void deactivate();
    bool isActive() const;

    static QDir effectiveRootDir();
    static QString map(const QString &fileName);

    bool addWorldFiles(const QString &directory, QDir::Filters filters = QDir::NoFilter,
        const QStringList &filterNames = QStringList(), bool recurse = true);
    bool addWorldFile(const QString &file);

  private:
    bool prepare();
    static QString createTmpDir(const QString &nameTemplate);
    static bool copyDir(const QString &directory, const QString &newName);

  private:
    static Sandbox *s_activeInstance;
    const bool m_defaultConstructed;
    const Usage m_usage;
    const Scopes m_scopes;
    const QString m_sandboxPath;
    bool m_prepared;
    QString m_tempDir;
    QDir m_workingSandboxDir;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Sandbox::Scopes)

#endif
