TARGET = ut_sandbox
include(../testapplication.pri)
include(../../libssu/libssu.pri)

SOURCES = \
        sandboxtest.cpp \

test_data_world.path = $${TESTS_DATA_PATH}/world
test_data_world.files = \
        testdata/world/world-only \
        testdata/world/world-and-sandbox \
        testdata/world/world-only-to-be-copied-into-sandbox \

test_data_sandbox.path = $${TESTS_DATA_PATH}/sandbox/$${test_data_world.path}
test_data_sandbox.files = \
        testdata/sandbox/world-and-sandbox \
        testdata/sandbox/sandbox-only \

INSTALLS += test_data_world test_data_sandbox
