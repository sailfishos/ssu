/**
 * @file rndssucli.h
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#ifndef _RndSsuCli_H
#define _RndSsuCli_H

#include <QObject>
#include <QSettings>
#include <QDebug>

#include <ssu.h>

class RndSsuCli: public QObject {
    Q_OBJECT

  public:
    RndSsuCli();

  public slots:
    void run();

  private:
    Ssu ssu;
    QSettings settings;
    void usage();
    void optFlavour(QString newFlavour="");
    void optRegister();
    void optResolve();
    void optStatus();
    void optUpdate(bool force=false);


  private slots:
    void handleResponse();

  signals:
    void done();

};

#endif
