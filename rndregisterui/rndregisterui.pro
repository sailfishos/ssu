TARGET = rndregisterui
include(../ssuapplication.pri)
include(rndregisterui_dependencies.pri)

HEADERS = rndregisterui.h
SOURCES = main.cpp \
        rndregisterui.cpp
RESOURCES = resources.qrc

CONFIG += qdeclarative-boostable
QT += gui declarative

OTHER_FILES += \
            resources/qml/rndregisterui.qml \
            resources/qml/RndSsuPage.qml

desktop.files = rndregisterui.desktop
desktop.path  = /usr/share/applications/

INSTALLS += desktop
