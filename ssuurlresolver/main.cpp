/**
 * @file main.cpp
 * @copyright 2012 Jolla Ltd.
 * @author Bernd Wachter <bernd.wachter@jollamobile.com>
 * @date 2012
 */

#include <QCoreApplication>
#include <QTimer>
#include "ssuurlresolver.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Jolla");
    QCoreApplication::setOrganizationDomain("http://www.jollamobile.com");
    QCoreApplication::setApplicationName("SSU Url Resolver");

    SsuUrlResolver mw;
    QTimer::singleShot(0, &mw, SLOT(run()));

    return app.exec();
}
