TARGET = ssu
include(../ssuapplication.pri)
include(ssuurlresolver_dependencies.pri)

# We do not build a typical application - override defaults from ../ssuapplication.pri
DESTDIR = $$DESTDIR_LIB/zypp/plugins/urlresolver
target.path = $$[QT_INSTALL_PREFIX]/libexec/zypp/plugins/urlresolver

QT += network
CONFIG += link_pkgconfig

# Needed for recent versions of libzypp
QMAKE_CXXFLAGS += -std=c++11

HEADERS = ssuurlresolver.h
SOURCES = main.cpp \
        ssuurlresolver.cpp

CONFIG += link_pkgconfig
PKGCONFIG += libzypp libsystemd
