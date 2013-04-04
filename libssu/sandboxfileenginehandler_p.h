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
    QAbstractFileEngine *create(const QString &fileName) const;
};

#endif
