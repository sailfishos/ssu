TARGET = ut_ssuurlresolver
include(../testapplication.pri)
include(ut_ssuurlresolver_dependencies.pri)

CONFIG += link_pkgconfig
PKGCONFIG += libzypp

HEADERS = \
        ssuurlresolvertest.h \

SOURCES = \
        main.cpp \
        ssuurlresolvertest.cpp \

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
