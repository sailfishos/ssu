/**
 * @file ssulog_p.h
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#ifndef _SSULOG_P_H
#define _SSULOG_P_H

#include <QObject>

#include <systemd/sd-journal.h>

namespace SsuLog
{
    /**
     * Print a message to systemds journal, or to a text log file, if a fallback is defined
     */
    void print(int priority, const QString &message);
}

#endif
