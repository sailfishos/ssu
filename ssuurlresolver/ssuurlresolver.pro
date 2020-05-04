TARGET = ssu
include(../ssuapplication.pri)
include(ssuurlresolver_dependencies.pri)

# We do not build a typical application - override defaults from ../ssuapplication.pri
DESTDIR = $$DESTDIR_LIB/zypp/plugins/urlresolver
target.path = /usr/lib/zypp/plugins/urlresolver

QT += network
CONFIG += link_pkgconfig

# Needed for recent versions of libzypp
QMAKE_CXXFLAGS += -std=c++11

HEADERS = ssuurlresolver.h
SOURCES = main.cpp \
        ssuurlresolver.cpp

CONFIG += link_pkgconfig
PKGCONFIG += libzypp libsystemd
