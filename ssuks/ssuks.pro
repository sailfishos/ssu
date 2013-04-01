HEADERS = ssuks.h \
        ssukickstarter.h
SOURCES = ssuks.cpp \
          ssukickstarter.cpp
TEMPLATE = app
TARGET = ssuks
LIBS += -lssu
CONFIG -= app_bundle
CONFIG += console
QT -= gui
QT += network

unix:target.path = $${PREFIX}/usr/bin
INSTALLS += target

!include( ../buildpath.pri ) { error("Unable to find build path specification") }
