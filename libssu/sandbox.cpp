/**
 * @file sandbox.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#include "sandbox_p.h"

#include <stdlib.h>

#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFileInfo>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QSet>

#include "libssu/ssucoreconfig.h"
#include "constants.h"

/**
 * @class Sandbox
 * @brief Helps to redirect file operations into sandbox directory
 *
 * The term <em>world files</em> is used to reffer files outside sandbox.
 *
 * To write a sandbox aware code, simply use the map() function to process paths
 * before accessing them.
 *
 * @code
 * QFile data(Sandbox::map("/usr/share/my_app/data"));
 * data.open();
 * ...
 * @endcode
 *
 * Its effect is controlled by activate() and deactivate() calls. Only one
 * Sandbox instance can be active at any time. Active sandbox is automatically
 * deactivated upon destruction.
 *
 * When constructed without arguments, path to sandbox directory is get from
 * @c SSU_SANDBOX_DIR environment variable.
 *
 * @attention When constructed without arguments, it is activated automatically
 * and failure to do so is reported with @c qFatal(), i.e., application will be
 * abort()ed.
 *
 * When constructed with @a usage UseAsSkeleton, it will first make temporary
 * copy of @a sandboxPath to work on and files in the original directory will
 * stay untouched.  Also see addWorldFiles().
 *
 * The argument @scopes allows to control if the sandbox will be used by this
 * process, its children processes (@c SSU_SANDBOX_DIR environment variable
 * will be exported), or both.
 */

Sandbox *Sandbox::s_activeInstance = 0;

Sandbox::Sandbox()
  : m_defaultConstructed(true), m_usage(UseDirectly), m_scopes(ThisProcess),
    m_sandboxPath(QProcessEnvironment::systemEnvironment().value("SSU_SANDBOX_DIR")),
    m_prepared(false){
  if (!activate()){
    qFatal("%s: Failed to activate", Q_FUNC_INFO);
  }
}

Sandbox::Sandbox(const QString &sandboxPath, Usage usage, Scopes scopes)
  : m_defaultConstructed(false), m_usage(usage), m_scopes(scopes),
    m_sandboxPath(sandboxPath), m_prepared(false){
  Q_ASSERT(!sandboxPath.isEmpty());
}

Sandbox::~Sandbox(){
  if (isActive()){
    deactivate();
  }

  if (!m_tempDir.isEmpty() && QFileInfo(m_tempDir).exists()){
    if (QProcess::execute("rm", QStringList() << "-rf" << m_tempDir) != 0){
      qWarning("%s: Failed to remove temporary directory", Q_FUNC_INFO);
    }
  }
}

bool Sandbox::isActive() const{
  return s_activeInstance == this;
}

bool Sandbox::activate(){
  Q_ASSERT_X(s_activeInstance == 0, Q_FUNC_INFO, "Only one instance can be active!");

  if (!prepare()){
    return false;
  }

  if (m_scopes & ChildProcesses){
    setenv("SSU_SANDBOX_DIR", qPrintable(m_workingSandboxDir.path()), 1);
  }

  s_activeInstance = this;
  return true;
}

void Sandbox::deactivate(){
  Q_ASSERT(isActive());

  if (m_scopes & ChildProcesses){
    unsetenv("SSU_SANDBOX_DIR");
  }

  s_activeInstance = 0;
}

QDir Sandbox::effectiveRootDir()
{
  return s_activeInstance != 0 && s_activeInstance->m_scopes & ThisProcess
    ? s_activeInstance->m_workingSandboxDir
    : QDir::root();
}

QString Sandbox::map(const QString &fileName)
{
  return effectiveRootDir().filePath(
      QDir::root().relativeFilePath(
        QFileInfo(fileName).absoluteFilePath()));
}

/**
 * Copies selected files into sandbox. Existing files in sandbox are not overwriten.
 *
 * @c QDir::NoDotAndDotDot is always added into @a filters.
 */
bool Sandbox::addWorldFiles(const QString &directory, QDir::Filters filters,
    const QStringList &filterNames, bool recurse){
  Q_ASSERT(!isActive());
  Q_ASSERT(!directory.isEmpty());

  if (!prepare()){
    return false;
  }

  const QString sandboxedDirectory = m_workingSandboxDir.filePath(
      QDir::root().relativeFilePath(
        QFileInfo(directory).absoluteFilePath()));

  if (!QFileInfo(directory).exists()){
    // Accept missing world directory - allow to create directories inside sandbox
    qDebug("%s: Directory does not exist - an empty one will be created instead of copied: '%s'",
        Q_FUNC_INFO, qPrintable(directory));
  } else if (!QFileInfo(directory).isDir()){
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

    const QFileInfo sandboxEntryInfo(QDir(sandboxedDirectory).filePath(worldEntryInfo.fileName()));

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

      if (recurse){
        if (!addWorldFiles(worldEntryInfo.absoluteFilePath(), filters, filterNames, true)){
          return false;
        }
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

  return true;
}

bool Sandbox::addWorldFile(const QString &file){
  return addWorldFiles(QFileInfo(file).path(), QDir::NoFilter,
      QStringList() << QFileInfo(file).fileName());
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
    if (m_tempDir = createTmpDir("ssu-sandbox.%1"), m_tempDir.isEmpty()){
      qWarning("%s: Failed to create sandbox directory", Q_FUNC_INFO);
      return false;
    }

    const QString sandboxCopyPath = QDir(m_tempDir).filePath("configroot");

    if (!copyDir(m_sandboxPath, sandboxCopyPath)){
      qWarning("%s: Failed to copy sandbox directory", Q_FUNC_INFO);
      return false;
    }

    m_workingSandboxDir = QDir(sandboxCopyPath);
  } else{
    m_workingSandboxDir = QDir(m_sandboxPath);
  }

  m_prepared = true;
  return true;
}

QString Sandbox::createTmpDir(const QString &nameTemplate){
  static const int REASONABLE_REPEAT_COUNT = 10;

  for (int i = 0; i < REASONABLE_REPEAT_COUNT; ++i){
    QString path;
    int suffix = 0;
    do{
      path = QDir::temp().filePath(nameTemplate.arg(++suffix));
    }while(QFileInfo(path).exists());

    if (QDir().mkpath(path)){
      return path;
    }
  }

  qWarning("%s: Failed to create temporary directory", Q_FUNC_INFO);
  return QString();
}

bool Sandbox::copyDir(const QString &directory, const QString &newName){
  const QDir sourceRoot(directory);
  const QDir destinationRoot(newName);

  QDirIterator it(directory, QDir::AllEntries|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
  while (it.hasNext()){
    it.next();

    const QFileInfo destination = QFileInfo(destinationRoot.filePath(
          sourceRoot.relativeFilePath(it.filePath())));

    if (it.fileInfo().isDir()){
      if (!QDir().mkpath(destination.absoluteFilePath())){
        qWarning("%s: Failed to mkpath '%s'", Q_FUNC_INFO, qPrintable(destination.absolutePath()));
        return false;
      }
    } else if (it.fileInfo().isFile()){
      if (!QDir().mkpath(destination.absolutePath())){
        qWarning("%s: Failed to mkpath '%s'", Q_FUNC_INFO, qPrintable(destination.absolutePath()));
        return false;
      }

      if (!QFile::copy(it.fileInfo().absoluteFilePath(), destination.absoluteFilePath())){
        qWarning("%s: Failed to copy file '%s'", Q_FUNC_INFO, qPrintable(it.filePath()));
        return false;
      }
    } else{
      qWarning("%s: Cannot copy other than regular files: '%s'", Q_FUNC_INFO,
          qPrintable(it.filePath()));
      return false;
    }
  }

  return true;
}
