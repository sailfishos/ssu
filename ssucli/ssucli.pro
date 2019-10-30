TARGET = ssu
include(../ssuapplication.pri)
include(ssucli_dependencies.pri)

QT += network dbus
CONFIG += link_pkgconfig

HEADERS = ssucli.h
SOURCES = main.cpp \
        ssucli.cpp
