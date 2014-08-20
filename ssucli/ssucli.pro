TARGET = ssu
include(../ssuapplication.pri)
include(ssucli_dependencies.pri)

QT += network dbus
CONFIG += link_pkgconfig

HEADERS = ssucli.h \
        ssuproxy.h
SOURCES = main.cpp \
        ssucli.cpp \
        ssuproxy.cpp
