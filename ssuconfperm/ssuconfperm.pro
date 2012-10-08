TEMPLATE = app
TARGET   = ssuconfperm
SOURCES  = ssuconfperm.c
HEADERS  = ../constants.h
CONFIG  -= qt

!include(../buildpath.pri){ error("Unable to find build path configuration")}

unix:target.path = $${PREFIX}/usr/bin
INSTALLS += target
