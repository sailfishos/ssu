/**
 * @file main.cpp
 * @copyright 2013 Jolla Ltd.
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2013
 */

#include <QCoreApplication>
#include "ssud.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Jolla");
    QCoreApplication::setOrganizationDomain("http://www.jollamobile.com");
    QCoreApplication::setApplicationName("ssud");

    Ssud ssud;

    app.exec();
}
