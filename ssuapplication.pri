isEmpty(TARGET):error("TARGET must be set before this file is included")
include(ssu_common.pri)

TEMPLATE = app
DESTDIR = $$DESTDIR_BIN

target.path = /usr/bin
INSTALLS += target
