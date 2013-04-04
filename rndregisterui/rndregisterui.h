/**
 * @file rndregisterui.h
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#ifndef RndRegisterUi_H
#define RndRegisterUi_H

#include <QMainWindow>
#include <QtDeclarative/QDeclarativeView>

#include "libssu/ssu.h"

namespace Ui {
  class RndRegisterUi;
}

class RndRegisterUi: public QMainWindow{
    Q_OBJECT

  public:
    explicit RndRegisterUi(QWidget *parent = 0);
    ~RndRegisterUi();

  private:
    Ssu ssu;
    QDeclarativeView *ui;
};

#endif
