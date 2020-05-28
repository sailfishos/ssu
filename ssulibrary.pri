isEmpty(TARGET):error("TARGET must be set before this file is included")
include(ssu_common.pri)

TEMPLATE = lib

DESTDIR = $$DESTDIR_LIB

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target

install_headers.path = /usr/include
INSTALLS += install_headers
