BUILD = ../build/libssu
HEADERS = ssu.h \
        ssudeviceinfo.h \
        ssulog.h \
        ssuvariables.h \
        ssusettings.h \
        ../constants.h
SOURCES = ssu.cpp \
        ssudeviceinfo.cpp \
        ssulog.cpp \
        ssuvariables.cpp \
        ssusettings.cpp
TEMPLATE = lib
TARGET = ssu
CONFIG += dll mobility link_pkgconfig
QT -= gui
QT += network xml
MOBILITY += systeminfo
PKGCONFIG += libsystemd-journal

headers.files = ssu.h
headers.path  = /usr/include

unix:target.path = $${PREFIX}/usr/lib
INSTALLS += target headers

ssuconfhack {
    DEFINES += SSUCONFHACK
}

!include( ../buildpath.pri ) { error("Unable to find build path specification") }
