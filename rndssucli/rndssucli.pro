TARGET = ssu
include(../ssuapplication.pri)
include(rndssucli_dependencies.pri)

QT += network dbus
CONFIG += link_pkgconfig
PKGCONFIG += connman-qt5

HEADERS = rndssucli.h \
        ssuproxy.h
SOURCES = main.cpp \
        rndssucli.cpp \
        ssuproxy.cpp
