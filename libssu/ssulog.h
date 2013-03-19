/**
 * @file ssulog.h
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#ifndef _SSULOG_H
#define _SSULOG_H

#include <QObject>

#include <systemd/sd-journal.h>

class SsuLog {

  public:
    static SsuLog *instance();
    /**
     * Print a message to systemds journal, or to a text log file, if a fallback is defined
     */
    void print(int priority, QString message);

  private:
    SsuLog() {};
    SsuLog(const SsuLog &); // hide copy constructor

    static SsuLog *ssuLog;
    QString fallbackLogPath;
};


#endif
