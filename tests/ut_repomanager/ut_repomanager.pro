TARGET = ut_repomanager
include(../testapplication.pri)
include(ut_repomanager_dependencies.pri)

HEADERS = \
        repomanagertest.h \

SOURCES = \
        main.cpp \
        repomanagertest.cpp \

test_data_etc.files = \
	testdata/ssu.ini \

test_data_usr_share.files = \
	testdata/ssu-defaults.ini \
