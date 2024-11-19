TARGET = ut_featuremodel
include(../testapplication.pri)
include(../../libssu/libssu.pri)
include(../testutils/testutils.pri)

HEADERS = \
        featuremodeltest.h

SOURCES = \
        main.cpp \
        featuremodeltest.cpp

RESOURCES = testdata.qrc
