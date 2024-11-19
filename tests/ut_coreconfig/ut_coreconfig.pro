TARGET = ut_coreconfig
include(../testapplication.pri)
include(../../libssu/libssu.pri)
include(../testutils/testutils.pri)

HEADERS = \
        coreconfigtest.h \

SOURCES = \
        main.cpp \
        coreconfigtest.cpp \

test_data_etc.files = \
	testdata/ssu.ini \

test_data_usr_share.files = \
	testdata/ssu-defaults.ini \
