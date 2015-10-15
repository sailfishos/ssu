TARGET = ut_featuremodel
include(../testapplication.pri)
include(ut_featuremodel_dependencies.pri)

HEADERS = \
        featuremodeltest.h

SOURCES = \
        main.cpp \
        featuremodeltest.cpp

RESOURCES = testdata.qrc
