TARGET = ssu-testutils
include(../testlibrary.pri)

HEADERS = \
        process.h \

SOURCES = \
        process.cpp \

exec_wrapper.path = $$TESTS_PATH
exec_wrapper.files = runtest.sh
INSTALLS += exec_wrapper
