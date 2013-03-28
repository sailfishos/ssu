/**
 * @file ssurepomanager.h
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#ifndef _SSUREPOMANAGER_H
#define _SSUREPOMANAGER_H

#include <QObject>
#include <QSettings>
#include <QHash>

class SsuRepoManager: public QObject {
    Q_OBJECT

  public:
    SsuRepoManager();
    /**
     * Look up all variables in the specified configuration file section,
     * run them through the variable expander, and add them to the supplied
     * QHash
     */
    void update();
};

#endif
