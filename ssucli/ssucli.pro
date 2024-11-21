TARGET = ssu
include(../ssuapplication.pri)
include(../libssu/libssu.pri)
include(../libssunetworkproxy/libssunetworkproxy.pri)

QT += network dbus
CONFIG += link_pkgconfig

HEADERS = ssucli.h
SOURCES = main.cpp \
        ssucli.cpp
