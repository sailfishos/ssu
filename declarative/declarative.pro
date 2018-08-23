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

HEADERS =  declarativessufeaturemodel.h \
           declarativessudeviceinfo.h
SOURCES += plugin.cpp \
           declarativessufeaturemodel.cpp \
           declarativessudeviceinfo.cpp

qmldir.files = qmldir *.qml *.js plugins.qmltypes
qmldir.path = $$target.path

INSTALLS += target qmldir

qmltypes.commands = qmlplugindump -nonrelocatable Nemo.Ssu 1.0 > $$PWD/plugins.qmltypes
QMAKE_EXTRA_TARGETS += qmltypes
