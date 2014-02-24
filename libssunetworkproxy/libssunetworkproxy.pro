TARGET = ssunetworkproxy
include(../ssulibrary.pri)

HEADERS = ssunetworkproxy.h

SOURCES = ssunetworkproxy.cpp

CONFIG += link_pkgconfig plugin

QT += network

PKGCONFIG += connman-qt5
