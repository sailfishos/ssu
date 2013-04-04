/**
 * @file sandboxfileenginehandler.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#include "sandboxfileenginehandler_p.h"

#include <QtCore/QDir>
#include <QtCore/QFSFileEngine>
#include <QtCore/QFileInfo>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QSet>

#include "libssu/ssucoreconfig.h"
// TODO: rename to ssuconstants.h?
#include "constants.h"

/**
 * @class SandboxFileEngineHandler
 *
 * Redirects all file operations on system configuration files to files under
 * directory specified by SSU_TESTS_SANDBOX environment variable.
 */

QSet<QString> SandboxFileEngineHandler::s_ssuConfigFiles = QSet<QString>()
  << SSU_CONFIGURATION
  << SSU_REPO_CONFIGURATION
  << SSU_DEFAULT_CONFIGURATION
  << SSU_BOARD_MAPPING_CONFIGURATION;

QSet<QString> SandboxFileEngineHandler::s_ssuConfigDirectories = QSet<QString>()
  << SSU_BOARD_MAPPING_CONFIGURATION_DIR;

SandboxFileEngineHandler::SandboxFileEngineHandler(){
  m_enabled = false;
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

  m_enabled = true;
}

SandboxFileEngineHandler::SandboxFileEngineHandler(const QString &sandboxPath){
  m_enabled = false;
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

  m_enabled = true;
}

QAbstractFileEngine *SandboxFileEngineHandler::create(const QString &fileName) const{
  Q_ASSERT(!m_enabled || !m_sandboxPath.isEmpty());

  if (!m_enabled){
    return 0;
  }

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
