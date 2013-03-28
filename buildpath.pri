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

LIBS += -L$$PWD/build/libssu
LD_LIBRARY_PATH = $$PWD/build/libssu
INCLUDEPATH += $$PWD/libssu
