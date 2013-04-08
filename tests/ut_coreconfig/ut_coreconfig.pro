TARGET = ut_coreconfig
include(../testapplication.pri)
include(ut_coreconfig_dependencies.pri)

HEADERS = \
        coreconfigtest.h \

SOURCES = \
        main.cpp \
        coreconfigtest.cpp \

test_data_etc.files = \
	testdata/ssu.ini \

test_data_usr_share.files = \
	testdata/ssu-defaults.ini \
