TARGET = ut_repomanager
include(../testapplication.pri)
include(../../libssu/libssu.pri)

SOURCES = \
        repomanagertest.cpp \

test_data_etc.files = \
        testdata/ssu.ini \

test_data_usr_share.files = \
        testdata/ssu-defaults.ini \

test_data_usr_share_repos_d.files = \
        testdata/repos.ini \
