/**
 * @file sandboxfileenginehandler.h
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#ifndef _SANDBOXINGFILEENGINEHANDLER_H
#define _SANDBOXINGFILEENGINEHANDLER_H

#include <QtCore/QAbstractFileEngineHandler>
#include <QtCore/QDir>
#include <QtCore/QFSFileEngine>
#include <QtCore/QFileInfo>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QSet>

#include <ssucoreconfig.h>
#include "../../constants.h"

/**
 * Redirects all file operations on system configuration files to files under
 * directory specified by SSU_TESTS_SANDBOX environment variable.
 */
class SandboxFileEngineHandler : public QAbstractFileEngineHandler {
  public:
    QAbstractFileEngine *create(const QString &fileName) const{
      static bool enabled = false;
      static bool firstCall = true;

      if (!enabled && !firstCall){
        return 0;
      }

      static QString sandboxPath =
          QProcessEnvironment::systemEnvironment().value("SSU_TESTS_SANDBOX");

      if (firstCall){
        firstCall = false;

        if (sandboxPath.isEmpty()){
          return 0;
        }

        if (!QFileInfo(sandboxPath).exists()){
          qFatal("%s: Invalid SSU_TESTS_SANDBOX value: No such file or directory",
              qPrintable(sandboxPath));
        }

        if (!QFileInfo(sandboxPath).isDir()){
          qFatal("%s: Invalid SSU_TESTS_SANDBOX value: Not a directory",
              qPrintable(sandboxPath));
        }

        enabled = true;
      }

      if (!fileName.startsWith('/')){
        return 0;
      }

      static QSet<QString> ssuConfigFiles = QSet<QString>()
        << SSU_CONFIGURATION
        << SSU_REPO_CONFIGURATION
        << SSU_DEFAULT_CONFIGURATION
        << SSU_BOARD_MAPPING_CONFIGURATION;

      static QSet<QString> ssuConfigDirectories = QSet<QString>()
        << SSU_BOARD_MAPPING_CONFIGURATION_DIR;

      if (!ssuConfigFiles.contains(fileName)){
        bool match = false;
        foreach (const QString &ssuConfigDirectory, ssuConfigDirectories){
          if (fileName.startsWith(ssuConfigDirectory + '/')){
            match = true;
            break;
          }
        }
        if (!match){
          return 0;
        }
      }

      const QString fileName_ = QDir(sandboxPath).absoluteFilePath(QString(fileName).remove(0, 1));

      return new QFSFileEngine(fileName_);
    }
};

#endif
