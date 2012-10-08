/**
 * @file main.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#include <QApplication>
#include "rndregisterui.h"

Q_DECL_EXPORT int main(int argc, char** argv){
  QApplication app(argc, argv);
  QCoreApplication::setOrganizationName("Jolla");
  QCoreApplication::setOrganizationDomain("http://www.jollamobile.com");
  QCoreApplication::setApplicationName("rndregisterui");

  RndRegisterUi mw;
  mw.showFullScreen();

  return app.exec();
}
