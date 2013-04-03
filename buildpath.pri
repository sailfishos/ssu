isEmpty(BUILD){
        isEmpty(TARGETDIR){
                BUILD = $$PWD/build/$$TARGET
        } else {
                BUILD = $$PWD/build/$$TARGETDIR
        }
}
isEmpty(DESTDIR){
        DESTDIR = $$BUILD
}

OBJECTS_DIR = $$BUILD
MOC_DIR = $$BUILD
UI_DIR = $$BUILD
UI_HEADERS_DIR = $$BUILD
UI_SOURCES_DIR = $$BUILD
RCC_DIR = $$BUILD

# This definitely needs to be refactorized somehow (the common module{.pro,.pri,_dependencies.pri}
# approach would do a good job here)
LIBS += -L$$PWD/build/libssu
LIBS += -L$$PWD/build/testutils
LD_LIBRARY_PATH = $$PWD/build/libssu:$$PWD/build/testutils
INCLUDEPATH += $$PWD/libssu
