HEADERS = \
	ssuurlresolvertest.h \

SOURCES = \
        main.cpp \
	ssuurlresolvertest.cpp \

TEMPLATE = app
TARGET = ut_ssuurlresolver

LIBS += \
	-lssu \
	-lzypp \

CONFIG -= app_bundle
CONFIG += console qtestlib
QT -= gui
QT += network testlib

!include( ../tests.pri ) { error("Unable to find tests include") }

unix:target.path = $${PREFIX}/$$TESTS_PATH
INSTALLS += target

test_data_etc.path = $${TESTS_DATA_PATH}/configroot/etc/ssu
test_data_etc.files = \
	$${PWD}/testdata/ssu.ini \

test_data_usr_share.path = $${TESTS_DATA_PATH}/configroot/usr/share/ssu
test_data_usr_share.files = \
	$${PWD}/testdata/ssu-defaults.ini \
	$${PWD}/testdata/repos.ini \
	$${PWD}/testdata/board-mappings.ini \

INSTALLS += test_data_etc test_data_usr_share

!include( ../../buildpath.pri ) { error("Unable to find build path specification") }
