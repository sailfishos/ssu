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
    FileEngineHandler(const QString &sandboxPath, const QSet<QString> &overlayEnabledDirectories);

    bool isDirectoryOverlayEnabled(const QString &path) const;

  public:
    // QAbstractFileEngineHandler
    QAbstractFileEngine *create(const QString &fileName) const;

  private:
    const QString m_sandboxPath;
    QSet<QString> m_overlayEnabledDirectories;
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

Sandbox *Sandbox::s_activeInstance = 0;

Sandbox::Sandbox()
  : m_defaultConstructed(true), m_usage(UseDirectly), m_scopes(ThisProcess),
    m_sandboxPath(QProcessEnvironment::systemEnvironment().value("SSU_TESTS_SANDBOX")),
    m_prepared(false), m_handler(0){
  if (!activate()){
    qFatal("%s: Failed to activate", Q_FUNC_INFO);
  }
}

Sandbox::Sandbox(const QString &sandboxPath, Usage usage, Scopes scopes)
  : m_defaultConstructed(false), m_usage(usage), m_scopes(scopes),
    m_sandboxPath(sandboxPath), m_prepared(false), m_handler(0){
  Q_ASSERT(!sandboxPath.isEmpty());
}

Sandbox::~Sandbox(){
  delete m_handler;

  if (!m_tempDir.isEmpty() && QFileInfo(m_tempDir).exists()){
    if (QProcess::execute("rm", QStringList() << "-rf" << m_tempDir) != 0){
      qWarning("%s: Failed to remove temporary directory", Q_FUNC_INFO);
    }
  }

  s_activeInstance = 0;
}

bool Sandbox::isActive() const{
  return s_activeInstance == this;
}

bool Sandbox::activate(){
  Q_ASSERT_X(s_activeInstance == 0, Q_FUNC_INFO, "Only one instance can be active!");

  if (!prepare()){
    return false;
  }

  if (m_scopes & ThisProcess){
    m_handler = new FileEngineHandler(m_workingSandboxPath, m_overlayEnabledDirectories);
  }

  if (m_scopes & ChildProcesses){
    setenv("SSU_TESTS_SANDBOX", qPrintable(m_workingSandboxPath), 1);
  }

  s_activeInstance = this;
  return true;
}

/**
 * Copies selected files into sandbox. Existing files in sandbox are not overwriten.
 *
 * @c QDir::NoDotAndDotDot is always added into @a filters.
 */
bool Sandbox::addWorldFiles(const QString &directory, QDir::Filters filters,
    const QStringList &filterNames){
  Q_ASSERT(!isActive());
  Q_ASSERT(!directory.isEmpty());
  Q_ASSERT(directory.startsWith('/'));
  Q_ASSERT(!directory.contains(':')); // separator in environment variable
  Q_ASSERT(!directory.contains("/./") && !directory.endsWith("/.")
      && !directory.contains("/../") && !directory.endsWith("/..")
      && !directory.contains("//"));
  Q_ASSERT_X(!(m_scopes & ChildProcesses), Q_FUNC_INFO, "Unimplemented case!");

  if (!prepare()){
    return false;
  }

  const QString sandboxedDirectory = m_workingSandboxPath + directory;

  if (!QFileInfo(directory).exists()){
    qWarning("%s: Directory does not exist: '%s'", Q_FUNC_INFO, qPrintable(directory));
    return false;
  }

  if (!QFileInfo(directory).isDir()){
    qWarning("%s: Is not a directory: '%s'", Q_FUNC_INFO, qPrintable(directory));
    return false;
  }

  if (!QFileInfo(sandboxedDirectory).exists()){
    if (!QDir().mkpath(sandboxedDirectory)){
      qWarning("%s: Failed to create sandbox directory '%s'", Q_FUNC_INFO,
          qPrintable(sandboxedDirectory));
      return false;
    }
  } else if (!QFileInfo(sandboxedDirectory).isDir()){
    qWarning("%s: Failed to create sandbox directory '%s': Is not a directory", Q_FUNC_INFO,
        qPrintable(sandboxedDirectory));
    return false;
  }

  if (filters == QDir::NoFilter){
    filters = QDir::AllEntries;
  }

  filters |= QDir::NoDotAndDotDot;

  foreach (const QFileInfo &worldEntryInfo, QDir(directory).entryInfoList(filterNames, filters)){

    const QFileInfo sandboxEntryInfo(sandboxedDirectory + '/' + worldEntryInfo.fileName());

    if (worldEntryInfo.isDir()){
      if (!sandboxEntryInfo.exists()){
        if (!QDir(sandboxedDirectory).mkdir(worldEntryInfo.fileName())){
          qWarning("%s: Failed to create overlay directory '%s/%s'", Q_FUNC_INFO,
              qPrintable(sandboxedDirectory), qPrintable(worldEntryInfo.fileName()));
          return false;
        }
      } else if (!sandboxEntryInfo.isDir()){
          qWarning("%s: Failed to create sandboxed copy '%s': Is not a directory", Q_FUNC_INFO,
              qPrintable(sandboxEntryInfo.filePath()));
          return false;
      }
    } else{
      if (!sandboxEntryInfo.exists()){
        if (!QFile(worldEntryInfo.filePath()).copy(sandboxEntryInfo.filePath())){
          qWarning("%s: Failed to copy file into sandbox '%s'", Q_FUNC_INFO,
              qPrintable(worldEntryInfo.filePath()));
          return false;
        }
      } else if (sandboxEntryInfo.isDir()){
          qWarning("%s: Failed to create sandboxed copy '%s': Is a directory", Q_FUNC_INFO,
              qPrintable(sandboxEntryInfo.filePath()));
          return false;
      }
    }
  }

  m_overlayEnabledDirectories.insert(directory);

  return true;
}

bool Sandbox::prepare(){
  Q_ASSERT(m_defaultConstructed || !m_sandboxPath.isEmpty());

  if (m_prepared){
    return true;
  }

  if (m_sandboxPath.isEmpty()){
    return true;
  }

  if (!QFileInfo(m_sandboxPath).exists()){
    qWarning("%s: Invalid sandboxPath: No such file or directory", qPrintable(m_sandboxPath));
    return false;
  }

  if (!QFileInfo(m_sandboxPath).isDir()){
    qWarning("%s: Invalid sandboxPath: Not a directory", qPrintable(m_sandboxPath));
    return false;
  }

  if (m_usage == UseAsSkeleton){
    QProcess mktemp;
    mktemp.start("mktemp", QStringList() << "-t" << "-d" << "ssu-sandbox.XXX");
    if (!mktemp.waitForFinished() || mktemp.exitCode() != 0){
      qWarning("%s: Failed to create sandbox directory", Q_FUNC_INFO);
      return false;
    }

    m_tempDir = mktemp.readAllStandardOutput().trimmed();
    if (!QFileInfo(m_tempDir).isDir()){
      qWarning("%s: Temporary directory disappeared: '%s'", Q_FUNC_INFO, qPrintable(m_tempDir));
      return false;
    }

    const QString sandboxCopyPath = QString("%1/configroot").arg(m_tempDir);

    if (QProcess::execute("cp", QStringList() << "-r" << m_sandboxPath << sandboxCopyPath) != 0){
      qWarning("%s: Failed to copy sandbox directory", Q_FUNC_INFO);
      return false;
    }

    m_workingSandboxPath = sandboxCopyPath;
  } else{
    m_workingSandboxPath = m_sandboxPath;
  }

  m_prepared = true;
  return true;
}

/*
 * @class Sandbox::FileEngineHandler
 */

Sandbox::FileEngineHandler::FileEngineHandler(const QString &sandboxPath,
    const QSet<QString> &overlayEnabledDirectories):
  m_sandboxPath(sandboxPath), m_overlayEnabledDirectories(overlayEnabledDirectories){
  Q_ASSERT(!sandboxPath.isEmpty());
}

bool Sandbox::FileEngineHandler::isDirectoryOverlayEnabled(const QString &path) const{
  return m_overlayEnabledDirectories.contains(path);
}

QAbstractFileEngine *Sandbox::FileEngineHandler::create(const QString &fileName) const{
  Q_ASSERT(!m_sandboxPath.isEmpty());

  if (!fileName.startsWith('/')){
    return 0;
  }

  const QString sandboxedFileName = m_sandboxPath + fileName;
  QScopedPointer<QFSFileEngine> sandboxedFileEngine(new QFSFileEngine(sandboxedFileName));

  const QAbstractFileEngine::FileFlags flags = sandboxedFileEngine->fileFlags(
      QAbstractFileEngine::ExistsFlag | QAbstractFileEngine::DirectoryType);

  if (!(flags & QAbstractFileEngine::ExistsFlag)){
    return 0;
  }

  if (flags & QAbstractFileEngine::DirectoryType){
    if (!isDirectoryOverlayEnabled(fileName)){
      return 0;
    }
  }

  return sandboxedFileEngine.take();
}
