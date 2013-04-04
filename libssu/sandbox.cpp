/**
 * @file sandbox.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#include "sandbox_p.h"

#include <stdlib.h>

#include <QtCore/QAbstractFileEngineHandler>
#include <QtCore/QDir>
#include <QtCore/QFSFileEngine>
#include <QtCore/QFileInfo>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QSet>

#include "libssu/ssucoreconfig.h"
// TODO: rename to ssuconstants.h?
#include "constants.h"

class Sandbox::FileEngineHandler : public QAbstractFileEngineHandler {
  public:
    FileEngineHandler(const QString &sandboxPath);

    QAbstractFileEngine *create(const QString &fileName) const;

  private:
    const QString m_sandboxPath;
};

/**
 * @class Sandbox
 *
 * Redirects all file operations on system configuration files to files under
 * sandbox directory. When constructed without arguments, the directory is get
 * from @c SSU_TESTS_SANDBOX environment variable.
 *
 * When constructed with @a usage UseAsSkeleton, it will first make temporary
 * copy of @a sandboxPath to work on and files in the original directory will
 * stay untouched.
 *
 * The argument @scopes allows to control if the sandbox will be used by this
 * process, its children processes (@c SSU_TESTS_SANDBOX environment variable
 * will be exported), or both.
 *
 * Internally it is based on QAbstractFileEngineHandler.
 */

Sandbox *Sandbox::s_instance = 0;

QSet<QString> Sandbox::s_ssuConfigFiles = QSet<QString>()
  << SSU_CONFIGURATION
  << SSU_REPO_CONFIGURATION
  << SSU_DEFAULT_CONFIGURATION
  << SSU_BOARD_MAPPING_CONFIGURATION;

QSet<QString> Sandbox::s_ssuConfigDirectories = QSet<QString>()
  << SSU_BOARD_MAPPING_CONFIGURATION_DIR;

Sandbox::Sandbox(){
  if (s_instance != 0){
    qFatal("%s: Cannot be instantiated more than once", Q_FUNC_INFO);
  }

  s_instance = this;

  m_handler = 0;

  m_sandboxPath = QProcessEnvironment::systemEnvironment().value("SSU_TESTS_SANDBOX");

  if (m_sandboxPath.isEmpty()){
    return;
  }

  if (!QFileInfo(m_sandboxPath).exists()){
    qFatal("%s: Invalid SSU_TESTS_SANDBOX value: No such file or directory",
        qPrintable(m_sandboxPath));
  }

  if (!QFileInfo(m_sandboxPath).isDir()){
    qFatal("%s: Invalid SSU_TESTS_SANDBOX value: Not a directory",
        qPrintable(m_sandboxPath));
  }

  m_handler = new FileEngineHandler(m_sandboxPath);
}

Sandbox::Sandbox(const QString &sandboxPath, Usage usage, Scopes scopes){
  if (s_instance != 0){
    qFatal("%s: Cannot be instantiated more than once", Q_FUNC_INFO);
  }

  s_instance = this;

  m_handler = 0;

  m_sandboxPath = sandboxPath;

  if (m_sandboxPath.isEmpty()){
    qWarning("%s: Empty sandboxPath", Q_FUNC_INFO);
    return;
  }

  if (!QFileInfo(m_sandboxPath).exists()){
    qFatal("%s: Invalid sandboxPath: No such file or directory",
        qPrintable(m_sandboxPath));
  }

  if (!QFileInfo(m_sandboxPath).isDir()){
    qFatal("%s: Invalid sandboxPath: Not a directory",
        qPrintable(m_sandboxPath));
  }

  if (usage == UseAsSkeleton){
    QProcess mktemp;
    mktemp.start("mktemp", QStringList() << "-t" << "-d" << "ssu-sandbox.XXX");
    if (!mktemp.waitForFinished() || mktemp.exitCode() != 0){
      qFatal("%s: Failed to create sandbox directory", Q_FUNC_INFO);
    }

    m_tempDir = mktemp.readAllStandardOutput().trimmed();
    if (!QFileInfo(m_tempDir).isDir()){
      qFatal("%s: Temporary directory disappeared: '%s'", Q_FUNC_INFO, qPrintable(m_tempDir));
    }

    const QString sandboxCopyPath = QString("%1/configroot").arg(m_tempDir);

    if (QProcess::execute("cp", QStringList() << "-r" << m_sandboxPath << sandboxCopyPath) != 0){
      qFatal("%s: Failed to copy sandbox directory", Q_FUNC_INFO);
    }

    m_sandboxPath = sandboxCopyPath;
  }

  if (scopes & ThisProcess){
    m_handler = new FileEngineHandler(m_sandboxPath);
  }

  if (scopes & ChildProcesses){
    setenv("SSU_TESTS_SANDBOX", qPrintable(m_sandboxPath), 1);
  }
}

Sandbox::~Sandbox(){
  delete m_handler;

  if (!m_tempDir.isEmpty() && QFileInfo(m_tempDir).exists()){
    if (QProcess::execute("rm", QStringList() << "-rf" << m_tempDir) != 0){
      qWarning("%s: Failed to remove temporary directory", Q_FUNC_INFO);
    }
  }

  s_instance = 0;
}

/*
 * @class Sandbox::FileEngineHandler
 */

Sandbox::FileEngineHandler::FileEngineHandler(const QString &sandboxPath):
  m_sandboxPath(sandboxPath){
  Q_ASSERT(!sandboxPath.isEmpty());
}

QAbstractFileEngine *Sandbox::FileEngineHandler::create(const QString &fileName) const{
  Q_ASSERT(!m_sandboxPath.isEmpty());

  if (!fileName.startsWith('/')){
    return 0;
  }

  if (!s_ssuConfigFiles.contains(fileName)){
    bool match = false;
    foreach (const QString &ssuConfigDirectory, s_ssuConfigDirectories){
      if (fileName.startsWith(ssuConfigDirectory + '/')){
        match = true;
        break;
      }
    }
    if (!match){
      return 0;
    }
  }

  const QString fileName_ = QDir(m_sandboxPath).absoluteFilePath(QString(fileName).remove(0, 1));

  return new QFSFileEngine(fileName_);
}
