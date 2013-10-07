TARGET = ssud
include(../ssuapplication.pri)
include(ssud_dependencies.pri)

QT += network dbus
CONFIG += link_pkgconfig
PKGCONFIG += connman-qt5

HEADERS = ssuadaptor.h \
        ssud.h
SOURCES = ssuadaptor.cpp \
        ssud.cpp \
        main.cpp

DBUS_SERVICE_NAME=org.nemo.ssu

service.files = ../dbus/$${DBUS_SERVICE_NAME}.service
service.path = /usr/share/dbus-1/system-services/

conf.files = ../dbus/$${DBUS_SERVICE_NAME}.conf
conf.path = /etc/dbus-1/system.d/

INSTALLS += service conf