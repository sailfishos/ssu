CONFIG -= gui

INCLUDEPATH *= $$PWD
DEPENDPATH *= $$PWD

# TODO: determine build dir automatically
SSU_BUILD_DIR = $$PWD/build

DESTDIR_BIN = $$SSU_BUILD_DIR/bin
DESTDIR_LIB = $$SSU_BUILD_DIR/lib

LIBS += -L$$DESTDIR_LIB
