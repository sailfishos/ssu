TARGET = ssud
include(../ssuapplication.pri)
include(ssud_dependencies.pri)

QT += network dbus
CONFIG += link_pkgconfig

HEADERS = \
        ssud.h \
        ssud_dbus.h
SOURCES = \
        ssud.cpp \
        main.cpp

DBUS_SERVICE_NAME=org.nemo.ssu

systemd.files = ../systemd/dbus-$${DBUS_SERVICE_NAME}.service
systemd.path = /usr/lib/systemd/system/

service.files = ../dbus/$${DBUS_SERVICE_NAME}.service
service.path = /usr/share/dbus-1/system-services/

conf.files = ../dbus/$${DBUS_SERVICE_NAME}.conf
conf.path = /etc/dbus-1/system.d/

INSTALLS += systemd service conf

ssu_dbus_adaptor.files = ../dbus/org.nemo.ssu.xml
ssu_dbus_adaptor.source_flags = -c SsuAdaptor
ssu_dbus_adaptor.header_flags = -c SsuAdaptor -i ssud/ssud_dbus.h
DBUS_ADAPTORS += ssu_dbus_adaptor
