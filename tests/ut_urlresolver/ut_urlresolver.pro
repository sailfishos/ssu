TARGET = ut_urlresolver
include(../testapplication.pri)
include(ut_urlresolver_dependencies.pri)

QT += xml

HEADERS = urlresolvertest.h
SOURCES = main.cpp \
        urlresolvertest.cpp

test_data.files = \
        testdata/mycert.crt \
        testdata/mykey.key \

test_data_etc.files = \
	testdata/ssu.ini \

test_data_usr_share.files = \
	testdata/ssu-defaults.ini \
	testdata/repos.ini \
	testdata/board-mappings.ini \
