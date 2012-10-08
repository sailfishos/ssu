HEADERS = rndregisterui.h
SOURCES = main.cpp \
        rndregisterui.cpp
RESOURCES = resources.qrc
TEMPLATE = app
TARGET = rndregisterui
LIBS += -lssu

CONFIG += qdeclarative-boostable
CONFIG -= app_bundle
QT += core gui declarative network

OTHER_FILES += \
            resources/qml/rndregisterui.qml \
            resources/qml/RndSsuPage.qml
unix:target.path = $${PREFIX}/usr/bin

desktop.files = rndregisterui.desktop
desktop.path  = /usr/share/applications/

INSTALLS += target desktop

!include( ../buildpath.pri ) { error("Unable to find build path specification") }
