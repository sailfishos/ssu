TARGET = ssuslipstream

include(../ssuapplication.pri)
include(ssuslipstream_dependencies.pri)

QT += network
CONFIG += link_pkgconfig

SOURCES += ssuslipstream.cpp
HEADERS += ssuslipstream.h

SOURCES += main.cpp

CONFIG += link_pkgconfig
PKGCONFIG += libsystemd-journal
