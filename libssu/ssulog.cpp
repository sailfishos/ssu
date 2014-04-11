/**
 * @file ssulog.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include <QFile>
#include <QTextStream>

#include "ssulog.h"
#include "ssucoreconfig.h"

SsuLog *SsuLog::ssuLog = 0;

SsuLog *SsuLog::instance(){
  if (!ssuLog){
    ssuLog = new SsuLog();
    ssuLog->fallbackLogPath = "/tmp/ssu.log";
    ssuLog->ssuLogLevel = -1;
  }

  return ssuLog;
}

void SsuLog::print(int priority, QString message){
  QByteArray ba = message.toUtf8();
  const char *ca = ba.constData();

  // directly go through qsettings here to avoid recursive invocation
  // of coreconfig / ssulog
  if (ssuLogLevel == -1){
    QSettings settings(SSU_CONFIGURATION, QSettings::IniFormat);

    if (settings.contains("loglevel"))
      ssuLog->ssuLogLevel = settings.value("loglevel").toInt();
    else
      ssuLog->ssuLogLevel = LOG_ERR;
  }

  // this is rather dirty, but since systemd does not seem to allow to enable debug
  // logging only for specific services probably best way for now
  if (priority > ssuLogLevel)
    return;

  if (sd_journal_print(priority, "ssu: %s", ca) < 0 && fallbackLogPath != ""){
    QFile logfile;
    QTextStream logstream;
    logfile.setFileName(fallbackLogPath);
    logfile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
    logstream.setDevice(&logfile);
    logstream << message << "\n";
    logstream.flush();
  }
}
