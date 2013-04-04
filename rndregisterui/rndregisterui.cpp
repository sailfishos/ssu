/**
 * @file rndregisterui.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#include <QDeclarativeContext>
#include <QDir>
#include <QGraphicsObject>
#include <QApplication>

#include "rndregisterui.h"

RndRegisterUi::RndRegisterUi(QWidget *parent): QMainWindow(parent){
  ui = new QDeclarativeView;

  ui->rootContext()->setContextProperty("ssu", &ssu);

  QDir dir;
  if (dir.exists("/home/nemo/rndregisterui.qml"))
    ui->setSource(QUrl("file:///home/nemo/rndregisterui.qml"));
  else
    ui->setSource(QUrl("qrc:/resources/qml/rndregisterui.qml"));

  setCentralWidget(ui);

  QObject *item=dynamic_cast<QObject*>(ui->rootObject());
  connect(item, SIGNAL(quit()), QApplication::instance(), SLOT(quit()));
}

RndRegisterUi::~RndRegisterUi(){
  delete ui;
}
