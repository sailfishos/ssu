TARGET = ut_deviceinfo
include(../testapplication.pri)
include(ut_deviceinfo_dependencies.pri)

HEADERS = \
        deviceinfotest.h \

SOURCES = \
        main.cpp \
        deviceinfotest.cpp \

test_data_etc.files = \
        testdata/ssu.ini \

test_data_usr_share.files = \
        testdata/ssu-defaults.ini

test_data_boardmappings_d.files = \
        testdata/board-mappings.ini \
