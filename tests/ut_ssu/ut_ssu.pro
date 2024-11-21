TARGET = ut_ssu
include(../testapplication.pri)
include(../../libssu/libssu.pri)

SOURCES = \
        ssutest.cpp \

test_data_etc.files = \
	testdata/ssu.ini \

test_data_usr_share_repos_d.files = \
	testdata/repos.ini \
