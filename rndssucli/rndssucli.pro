HEADERS = rndssucli.h
SOURCES = main.cpp \
        rndssucli.cpp
#RESOURCES = rndregister.qrc
TEMPLATE = app
TARGET = ssu
TARGETDIR = ssucli
LIBS += -lssu
CONFIG -= app_bundle
CONFIG += console
QT -= gui
QT += network

unix:target.path = $${PREFIX}/usr/bin
INSTALLS += target

!include( ../buildpath.pri ) { error("Unable to find build path specification") }
