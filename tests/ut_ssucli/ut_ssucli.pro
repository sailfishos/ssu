TARGET = ut_ssucli
include(../testapplication.pri)
include(../../libssu/libssu.pri)
include(../../sandboxhook/sandboxhook.pri)
include(../testutils/testutils.pri)

SOURCES = \
        ssuclitest.cpp \

test_data_etc.files = \
        testdata/ssu.ini \

test_data_usr_share.files = \
        testdata/ssu-defaults.ini \
        testdata/repos.ini \

test_data_boardmappings_d.files = \
        testdata/board-mappings.ini \
