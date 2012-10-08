HEADERS = urlresolvertest.h
SOURCES = main.cpp \
        urlresolvertest.cpp
TEMPLATE = app
TARGET = ut_urlresolver
LIBS += -lssu
CONFIG -= app_bundle
CONFIG += console qtestlib
QT -= gui
QT += network testlib

!include( ../tests.pri ) { error("Unable to find tests include") }

unix:target.path = $${PREFIX}/$$TESTS_PATH
INSTALLS += target

!include( ../../buildpath.pri ) { error("Unable to find build path specification") }
