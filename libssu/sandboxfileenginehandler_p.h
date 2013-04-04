/**
 * @file sandboxfileenginehandler_p.h
 * @copyright 2013 Jolla Ltd.
 * @author Martin Kampas <martin.kampas@tieto.com>
 * @date 2013
 */

#ifndef _SANDBOXINGFILEENGINEHANDLER_P_H
#define _SANDBOXINGFILEENGINEHANDLER_P_H

#include <QtCore/QAbstractFileEngineHandler>

class SandboxFileEngineHandler : public QAbstractFileEngineHandler {
  public:
    SandboxFileEngineHandler();
    SandboxFileEngineHandler(const QString &sandboxPath);

    QAbstractFileEngine *create(const QString &fileName) const;

  private:
    static QSet<QString> s_ssuConfigFiles;
    static QSet<QString> s_ssuConfigDirectories;
    bool m_enabled;
    QString m_sandboxPath;
};

#endif
