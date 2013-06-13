/**
 * @file ssuks.h
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#ifndef _SSUKS_H
#define _SSUKS_H

#include <QObject>
#include <QDebug>

class SsuKs: public QObject {
    Q_OBJECT

  public:
    SsuKs(){};

  public slots:
    void run();

  private:
    void usage();
};

#endif
