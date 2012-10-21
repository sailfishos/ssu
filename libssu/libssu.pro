BUILD = ../build/libssu
HEADERS = ssu.h \
        ../constants.h
SOURCES = ssu.cpp
TEMPLATE = lib
TARGET = ssu
CONFIG += dll mobility
QT -= gui
QT += network xml
MOBILITY += systeminfo

unix:target.path = $${PREFIX}/usr/lib
INSTALLS += target

ssuconfhack {
    DEFINES += SSUCONFHACK
}

!include( ../buildpath.pri ) { error("Unable to find build path specification") }
