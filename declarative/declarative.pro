TEMPLATE = lib
TARGET = declarativessu
TARGET = $$qtLibraryTarget($$TARGET)

include(../libssu/libssu.pri)
include(../ssu_common.pri)

MODULENAME = Nemo/Ssu
TARGETPATH = $$[QT_INSTALL_QML]/$$MODULENAME

QT += qml
QT -= gui

CONFIG += plugin

target.path = $$TARGETPATH

HEADERS =  declarativessufeaturemodel.h
SOURCES += plugin.cpp \
           declarativessufeaturemodel.cpp

qmldir.files = qmldir *.qml *.js
qmldir.path = $$target.path

INSTALLS += target qmldir
