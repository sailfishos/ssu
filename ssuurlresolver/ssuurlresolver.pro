HEADERS = ssuurlresolver.h
SOURCES = main.cpp \
        ssuurlresolver.cpp
TEMPLATE = app
TARGET = ssu
LIBS += -lssu
CONFIG -= app_bundle
CONFIG += console link_pkgconfig
QT -= gui
QT += network
PKGCONFIG += libzypp

unix:target.path = $${PREFIX}/usr/lib/zypp/plugins/urlresolver
INSTALLS += target

!include( ../buildpath.pri ) { error("Unable to find build path specification") }
