/**
 * @file ssulog.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include <QFile>
#include <QTextStream>

#include "ssulog_p.h"
#include "ssucoreconfig_p.h"

namespace {
int ssuLogLevel = -1 ;
QString fallbackLogPath = QStringLiteral("/tmp/ssu.log");
}

void SsuLog::print(int priority, const QString &message)
{
    // directly go through qsettings here to avoid recursive invocation
    // of coreconfig / ssulog
    if (ssuLogLevel == -1) {
        QSettings settings(SSU_CONFIGURATION, QSettings::IniFormat);

        if (settings.contains("loglevel"))
            ssuLogLevel = settings.value("loglevel").toInt();
        else
            ssuLogLevel = LOG_ERR;
    }

    // this is rather dirty, but since systemd does not seem to allow to enable debug
    // logging only for specific services probably best way for now
    if (priority > ssuLogLevel)
        return;

    QByteArray ba = message.toUtf8();
    const char *ca = ba.constData();

    if (sd_journal_print(priority, "ssu: %s", ca) < 0) {
        QFile logfile;
        QTextStream logstream;
        logfile.setFileName(fallbackLogPath);
        logfile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
        logstream.setDevice(&logfile);
        logstream << message << "\n";
        logstream.flush();
    }
}
