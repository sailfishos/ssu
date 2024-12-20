TARGET = ut_ssuurlresolver
include(../testapplication.pri)
include(../../libssu/libssu.pri)
include(../../sandboxhook/sandboxhook.pri)

CONFIG += link_pkgconfig
PKGCONFIG += libzypp

SOURCES = \
        ssuurlresolvertest.cpp \

test_data_etc.files = \
        testdata/ssu.ini \

test_data_usr_share.files = \
        testdata/ssu-defaults.ini \

test_data_usr_share_repos_d.files = \
        testdata/repos.ini \

test_data_boardmappings_d.files = \
        testdata/board-mappings.ini \
