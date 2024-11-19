TARGET = ssu
include(../ssuapplication.pri)
include(../libssu/libssu.pri)
include(../libssunetworkproxy/libssunetworkproxy.pri)

# We do not build a typical application - override defaults from ../ssuapplication.pri
DESTDIR = $$DESTDIR_LIB/zypp/plugins/urlresolver
target.path = $$[QT_INSTALL_PREFIX]/libexec/zypp/plugins/urlresolver

QT += network
CONFIG += link_pkgconfig

HEADERS = ssuurlresolver.h
SOURCES = main.cpp \
        ssuurlresolver.cpp

CONFIG += link_pkgconfig
PKGCONFIG += libzypp libsystemd
