TARGET = ut_rndssucli
include(../testapplication.pri)
include(ut_rndssucli_dependencies.pri)

HEADERS = \
        rndssuclitest.h \

SOURCES = \
        main.cpp \
        rndssuclitest.cpp \

test_data_etc.files = \
	testdata/ssu.ini \

test_data_usr_share.files = \
	testdata/ssu-defaults.ini \
	testdata/repos.ini \
	testdata/board-mappings.ini \
