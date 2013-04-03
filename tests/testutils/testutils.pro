HEADERS = \
        process.h \
        sandboxfileenginehandler.h \

SOURCES = \
        process.cpp \
        sandboxfileenginehandler.cpp \

TEMPLATE = lib
TARGET = testutils
CONFIG -= app_bundle
CONFIG += console qtestlib
QT -= gui
QT += network testlib

LIBS += -lssu

!include( ../tests.pri ) { error("Unable to find tests include") }

unix:target.path = $${PREFIX}/$$TESTS_PATH
INSTALLS += target

exec_wrapper.path = $${PREFIX}/$$TESTS_PATH
exec_wrapper.files = $${PWD}/runtest.sh
INSTALLS += exec_wrapper

!include( ../../buildpath.pri ) { error("Unable to find build path specification") }
