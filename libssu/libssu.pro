TARGET = ssu
include(../ssulibrary.pri)

ssu_dbus_interface.files = ../dbus/org.nemo.ssu.xml
ssu_dbus_interface.source_flags = -c SsuDBusInterface
ssu_dbus_interface.header_flags = -c SsuDBusInterface -i ssud/ssud_dbus.h
DBUS_INTERFACES += ssu_dbus_interface

# TODO: which headers are public? i.e. to be installed
public_headers = \
        ssu.h \
        ssudeviceinfo.h \
        ssurepomanager.h \
        ssufeaturemodel.h \
        ssuproxy.h

HEADERS = \
        $${public_headers} \
        sandbox_p.h \
        ssucoreconfig_p.h \
        ssufeaturemanager.h \
        ssulog_p.h \
        ssusettings_p.h \
        ssuvariables_p.h

SOURCES = \
        sandbox.cpp \
        ssu.cpp \
        ssucoreconfig.cpp \
        ssudeviceinfo.cpp \
        ssulog.cpp \
        ssufeaturemanager.cpp \
        ssufeaturemodel.cpp \
        ssuvariables.cpp \
        ssurepomanager.cpp \
        ssusettings.cpp \
        ssuproxy.cpp

CONFIG += link_pkgconfig
QT += network xml dbus
PKGCONFIG += libsystemd libshadowutils libzypp

install_headers.files = $${public_headers}

ssuconfhack {
    DEFINES += SSUCONFHACK
}

CONFIG += create_pc create_prl no_install_prl

pkgconfig.files = $$PWD/pkgconfig/ssu.pc
pkgconfig.path = $$[QT_INSTALL_LIBS]/pkgconfig

QMAKE_PKGCONFIG_NAME = $$TARGET
QMAKE_PKGCONFIG_DESCRIPTION = ssu development files
QMAKE_PKGCONFIG_VERSION = $$VERSION
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$install_headers.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_REQUIRES = Qt5Core

INSTALLS += pkgconfig
