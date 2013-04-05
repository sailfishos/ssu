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

    void enableDirectoryOverlay(const QString &path);
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

Sandbox *Sandbox::s_instance = 0;

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

bool Sandbox::isActive() const{
  return m_handler != 0;
}

/**
 * Copies selected files into sandbox. Existing files in sandbox are not overwriten.
 *
 * @c QDir::NoDotAndDotDot is always added into @a filters.
 */
void Sandbox::addWorldFiles(const QString &directory, QDir::Filters filters,
    const QStringList &filterNames){
  Q_ASSERT(!directory.isEmpty());
  Q_ASSERT(directory.startsWith('/'));
  Q_ASSERT(!directory.contains(':')); // separator in environment variable
  Q_ASSERT(!directory.contains("/./") && !directory.endsWith("/.")
      && !directory.contains("/../") && !directory.endsWith("/..")
      && !directory.contains("//"));
  Q_ASSERT_X(!(m_scopes & ChildProcesses), Q_FUNC_INFO, "Unimplemented case!");

  if (!isActive()){
    qDebug("%s: Sandbox is not active", Q_FUNC_INFO);
    return;
  }

  const QString sandboxedDirectory = m_sandboxPath + directory;

  QFSFileEngine worldDirectoryEngine(directory);
  QFSFileEngine sandboxDirectoryEngine(m_sandboxPath);
  QFSFileEngine sandboxedDirectoryEngine(sandboxedDirectory);

  const QAbstractFileEngine::FileFlags worldDirectoryFlags =
    worldDirectoryEngine.fileFlags(
        QAbstractFileEngine::ExistsFlag | QAbstractFileEngine::DirectoryType);

  if (!(worldDirectoryFlags & QAbstractFileEngine::ExistsFlag)){
    qDebug("%s: Directory does not exist: '%s'", Q_FUNC_INFO, qPrintable(directory));
    return;
  }

  if (!(worldDirectoryFlags & QAbstractFileEngine::DirectoryType)){
    qFatal("%s: Is not a directory: '%s'", Q_FUNC_INFO, qPrintable(directory));
  }

  const QAbstractFileEngine::FileFlags sandboxedDirectoryFlags =
    sandboxedDirectoryEngine.fileFlags(
        QAbstractFileEngine::ExistsFlag | QAbstractFileEngine::DirectoryType);

  if (!(sandboxedDirectoryFlags & QAbstractFileEngine::ExistsFlag)){
    if (!sandboxedDirectoryEngine.mkdir(sandboxedDirectory, true)){
      qFatal("%s: Failed to create sandbox directory '%s': %s", Q_FUNC_INFO,
          qPrintable(sandboxedDirectory), qPrintable(sandboxedDirectoryEngine.errorString()));
    }
  } else if (!(sandboxedDirectoryFlags & QAbstractFileEngine::DirectoryType)){
    qFatal("%s: Failed to create sandbox directory '%s': Is not a directory", Q_FUNC_INFO,
        qPrintable(sandboxedDirectory));
  }

  if (filters == QDir::NoFilter){
    filters = QDir::AllEntries;
  }

  filters |= QDir::NoDotAndDotDot;

  foreach (const QString &entryName, worldDirectoryEngine.entryList(filters, filterNames)){

    QFSFileEngine worldEntryEngine(directory + '/' + entryName);
    const QAbstractFileEngine::FileFlags worldEntryFlags = worldEntryEngine.fileFlags(
        QAbstractFileEngine::DirectoryType | QAbstractFileEngine::FileType);

    QFSFileEngine sandboxEntryEngine(sandboxedDirectory + '/' + entryName);
    const QAbstractFileEngine::FileFlags sandboxEntryFlags = sandboxEntryEngine.fileFlags(
        QAbstractFileEngine::ExistsFlag | QAbstractFileEngine::DirectoryType);

    if (worldEntryFlags & QAbstractFileEngine::DirectoryType){
      if (!(sandboxEntryFlags & QAbstractFileEngine::ExistsFlag)){
        if (!sandboxedDirectoryEngine.mkdir(entryName, false)){
          qFatal("%s: Failed to create overlay directory '%s/%s': %s", Q_FUNC_INFO,
              qPrintable(sandboxedDirectory), qPrintable(entryName),
              qPrintable(sandboxedDirectoryEngine.errorString()));
        }
      } else if (!(sandboxEntryFlags & QAbstractFileEngine::DirectoryType)){
          qFatal("%s: Failed to create sandboxed copy '%s/%s': Is not a directory", Q_FUNC_INFO,
              qPrintable(sandboxedDirectory), qPrintable(entryName));
      }
    } else if (worldEntryFlags & QAbstractFileEngine::FileType){
      if (!(sandboxEntryFlags & QAbstractFileEngine::ExistsFlag)){
        if (!copyFile(&worldEntryEngine, &sandboxEntryEngine)){
          return;
        }
      } else if (sandboxEntryFlags & QAbstractFileEngine::DirectoryType){
          qFatal("%s: Failed to create sandboxed copy '%s/%s': Is a directory", Q_FUNC_INFO,
              qPrintable(sandboxedDirectory), qPrintable(entryName));
      }
    } else{
      qFatal("%s: Failed to create sandboxed copy '%s/%s': "
          "Can only copy regular files and directories", Q_FUNC_INFO,
          qPrintable(sandboxedDirectory), qPrintable(entryName));
    }
  }

  m_handler->enableDirectoryOverlay(directory);
}

bool Sandbox::copyFile(QAbstractFileEngine *src, QAbstractFileEngine *dst){
  if (!src->open(QIODevice::ReadOnly)){
    qFatal("%s: Failed to create sandbox copy: '%s': Cannot open source file for reading",
        Q_FUNC_INFO, qPrintable(src->fileName()));
    return false;
  }

  if (!dst->open(QIODevice::ReadWrite)){
    qFatal("%s: Failed to create sandbox copy: '%s': Cannot open file for writing",
        Q_FUNC_INFO, qPrintable(dst->fileName()));
    return false;
  }

  char buf[4096];
  qint64 totalRead = 0;
  while (!src->atEnd()){
    qint64 read = src->read(buf, sizeof(buf));
    if (read <= 0){
      break;
    }
    totalRead += read;
    if (dst->write(buf, read) != read){
      qFatal("%s: Failed to create sandbox copy: '%s': Write error", Q_FUNC_INFO,
          qPrintable(src->fileName()));
      return false;
    }
  }

  if (totalRead != src->size()){
    qFatal("%s: Failed to create sandbox copy: '%s': Read/write error", Q_FUNC_INFO,
        qPrintable(src->fileName()));
      return false;
  }

  src->close();
  dst->close();

  return true;
}

/*
 * @class Sandbox::FileEngineHandler
 */

Sandbox::FileEngineHandler::FileEngineHandler(const QString &sandboxPath):
  m_sandboxPath(sandboxPath){
  Q_ASSERT(!sandboxPath.isEmpty());
}

void Sandbox::FileEngineHandler::enableDirectoryOverlay(const QString &path){
  m_overlayEnabledDirectories.insert(path);
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
