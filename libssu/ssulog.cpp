/**
 * @file ssulog.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include <QFile>
#include <QTextStream>

#include "ssulog.h"

SsuLog *SsuLog::ssuLog = 0;

SsuLog *SsuLog::instance(){
  if (!ssuLog){
    ssuLog = new SsuLog();
    ssuLog->fallbackLogPath = "/tmp/ssu.log";
  }

  return ssuLog;
}

void SsuLog::print(int priority, QString message){
  QByteArray ba = message.toUtf8();
  const char *ca = ba.constData();

  if (sd_journal_print(LOG_INFO, "ssu: %s", ca) < 0 && fallbackLogPath != ""){
    QFile logfile;
    QTextStream logstream;
    logfile.setFileName(fallbackLogPath);
    logfile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
    logstream.setDevice(&logfile);
    logstream << message << "\n";
    logstream.flush();
  }
}
