TARGET = ssu
include(../ssuapplication.pri)
include(rndssucli_dependencies.pri)

QT += network

HEADERS = rndssucli.h
SOURCES = main.cpp \
        rndssucli.cpp
#RESOURCES = rndregister.qrc
