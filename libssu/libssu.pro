TARGET = ssu
include(../ssulibrary.pri)

# TODO: which headers are public? i.e. to be installed
public_headers = \
        ssu.h \

HEADERS = \
        $${public_headers} \
        sandbox_p.h \
        ssucoreconfig.h \
        ssudeviceinfo.h \
        ssulog.h \
        ssuvariables.h \
        ssusettings.h \
        ssurepomanager.h \

SOURCES = \
        sandbox.cpp \
        ssu.cpp \
        ssucoreconfig.cpp \
        ssudeviceinfo.cpp \
        ssulog.cpp \
        ssuvariables.cpp \
        ssurepomanager.cpp \
        ssusettings.cpp

CONFIG += mobility link_pkgconfig
QT += network xml
MOBILITY += systeminfo
PKGCONFIG += libsystemd-journal

install_headers.files = $${public_headers}

ssuconfhack {
    DEFINES += SSUCONFHACK
}
