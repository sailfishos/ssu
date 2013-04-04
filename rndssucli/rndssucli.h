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
#include <QStringList>
#include <QDebug>

#include "libssu/ssu.h"

class RndSsuCli: public QObject {
    Q_OBJECT

  public:
    RndSsuCli();

  public slots:
    void run();

  private:
    Ssu ssu;
    QSettings settings;
    int state;
    void usage();
    void optFlavour(QStringList opt);
    void optMode(QStringList opt);
    void optModifyRepo(int action, QStringList opt);
    void optRegister();
    void optRelease(QStringList opt);
    void optRepos(QStringList opt);
    void optResolve(QStringList opt);
    void optStatus();
    void optUpdateCredentials(QStringList opt);
    void optUpdateRepos();

    enum Actions {
      Remove,
      Add,
      Disable,
      Enable
    };

    enum State {
      Idle,
      Busy,
      UserError
    };

  private slots:
    void handleResponse();

  signals:
    void done();

};

#endif
