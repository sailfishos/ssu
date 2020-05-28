include(../ssu_common.pri)
include(sandboxhook_dependencies.pri)

TEMPLATE = lib
TARGET = sandboxhook

CONFIG += unversioned_libname

SOURCES = sandboxhook.cpp

DESTDIR = $$DESTDIR_LIB

target.path = $$[QT_INSTALL_LIBS]/ssu
INSTALLS += target
