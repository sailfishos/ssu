TARGET = ut_ssu
include(../testapplication.pri)
include(ut_ssu_dependencies.pri)

HEADERS = \
        ssutest.h \

SOURCES = \
        main.cpp \
        ssutest.cpp \

test_data_etc.files = \
	testdata/ssu.ini \

test_data_usr_share_repos_d.files = \
	testdata/repos.ini \

