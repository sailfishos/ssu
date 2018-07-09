TARGET = ssu
include(../ssuapplication.pri)
include(ssucli_dependencies.pri)

QT += network dbus
CONFIG += link_pkgconfig

HEADERS = ssucli.h
SOURCES = main.cpp \
        ssucli.cpp

ssu_dbus_interface.files = ../dbus/org.nemo.ssu.xml
ssu_dbus_interface.source_flags = -c SsuProxy
ssu_dbus_interface.header_flags = -c SsuProxy -i ssud/ssud_include.h
DBUS_INTERFACES += ssu_dbus_interface
