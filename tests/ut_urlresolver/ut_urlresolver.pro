TARGET = ut_urlresolver
include(../testapplication.pri)
include(ut_urlresolver_dependencies.pri)

HEADERS = urlresolvertest.h
SOURCES = main.cpp \
        urlresolvertest.cpp

test_data_etc.files = \
	testdata/ssu.ini \

test_data_usr_share.files = \
	testdata/ssu-defaults.ini \
	testdata/repos.ini \
	testdata/board-mappings.ini \
