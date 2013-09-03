/**
 * @file main.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include <QCoreApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QNetworkProxyFactory>
#include "ssud.h"

int main(int argc, char** argv){
  QCoreApplication app(argc, argv);
  QCoreApplication::setOrganizationName("Jolla");
  QCoreApplication::setOrganizationDomain("http://www.jollamobile.com");
  QCoreApplication::setApplicationName("ssud");

  QTranslator qtTranslator;
  qtTranslator.load("qt_" + QLocale::system().name(),
                    QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app.installTranslator(&qtTranslator);

  QNetworkProxyFactory::setUseSystemConfiguration(true);

  Ssud ssud;

  app.exec();
}
