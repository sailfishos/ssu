TARGET = ssuslipstream

include(../ssuapplication.pri)
include(../libssu/libssu.pri)
include(../libssunetworkproxy/libssunetworkproxy.pri)

QT += network
CONFIG += link_pkgconfig

SOURCES += ssuslipstream.cpp
HEADERS += ssuslipstream.h

SOURCES += main.cpp

CONFIG += link_pkgconfig
PKGCONFIG += libsystemd
