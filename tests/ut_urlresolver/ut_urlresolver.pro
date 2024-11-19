TARGET = ut_urlresolver
include(../testapplication.pri)
include(../../libssu/libssu.pri)
include(../testutils/testutils.pri)

QT += xml

HEADERS = urlresolvertest.h
SOURCES = main.cpp \
        urlresolvertest.cpp

test_data.files = \
        testdata/mycert.crt \
        testdata/mykey.key \

test_data_etc.files = \
        testdata/ssu.ini \

test_data_usr_share.files = \
        testdata/ssu-defaults.ini \

test_data_usr_share_repos_d.files = \
        testdata/repos.ini \

test_data_boardmappings_d.files = \
        testdata/board-mappings.ini \
