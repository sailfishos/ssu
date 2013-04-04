TARGET = ut_ssuurlresolver
include(../testapplication.pri)
include(ut_ssuurlresolver_dependencies.pri)

CONFIG += link_pkgconfig
PKGCONFIG += libzypp

HEADERS = \
	ssuurlresolvertest.h \

SOURCES = \
        main.cpp \
	ssuurlresolvertest.cpp \

test_data_etc.files = \
	testdata/ssu.ini \

test_data_usr_share.files = \
	testdata/ssu-defaults.ini \
	testdata/repos.ini \
	testdata/board-mappings.ini \
