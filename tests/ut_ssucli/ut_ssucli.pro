TARGET = ut_ssucli
include(../testapplication.pri)
include(ut_ssucli_dependencies.pri)

HEADERS = \
        ssuclitest.h \

SOURCES = \
        main.cpp \
        ssuclitest.cpp \

test_data_etc.files = \
        testdata/ssu.ini \

test_data_usr_share.files = \
        testdata/ssu-defaults.ini \

test_data_usr_share_repos_d.files = \
        testdata/repos.ini \

test_data_boardmappings_d.files = \
        testdata/board-mappings.ini \

# Needed for recent versions of libzypp
QMAKE_CXXFLAGS += -std=c++11
