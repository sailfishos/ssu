/**
 * @file main.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#include <QCoreApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QTimer>
#include "ssucli.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Jolla");
    QCoreApplication::setOrganizationDomain("http://www.jollamobile.com");
    QCoreApplication::setApplicationName("ssu");

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    SsuCli mw;
    QTimer::singleShot(0, &mw, SLOT(run()));

    return app.exec();
}
