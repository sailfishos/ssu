TARGET = ssu
include(../ssulibrary.pri)

# TODO: which headers are public? i.e. to be installed
public_headers = \
        ssu.h \
        ssudeviceinfo.h \
        ssulog.h \
        ssurepomanager.h \
        ssusettings.h \
        ssuvariables.h

HEADERS = \
        $${public_headers} \
        sandbox_p.h \
        ssucoreconfig.h \
        mobility-booty/qofonoservice_linux_p.h \
        mobility-booty/qsysteminfo_linux_common_p.h \
        mobility-booty/qsysteminfo_dbus_p.h

SOURCES = \
        sandbox.cpp \
        ssu.cpp \
        ssucoreconfig.cpp \
        ssudeviceinfo.cpp \
        ssulog.cpp \
        ssuvariables.cpp \
        ssurepomanager.cpp \
        ssusettings.cpp \
        mobility-booty/qofonoservice_linux.cpp \
        mobility-booty/qsysteminfo_linux_common.cpp \

#CONFIG += mobility link_pkgconfig
CONFIG += link_pkgconfig
QT += network xml dbus
#MOBILITY += systeminfo
PKGCONFIG += libsystemd-journal boardname

install_headers.files = $${public_headers}

ssuconfhack {
    DEFINES += SSUCONFHACK
}
