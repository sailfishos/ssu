include(../ssuapplication.pri)
include(tests_common.pri)

QT += testlib

test_data.path = $${TESTS_DATA_PATH}
INSTALLS += test_data

test_data_etc.path = $${TESTS_DATA_PATH}/configroot/etc/ssu
test_data_usr_share.path = $${TESTS_DATA_PATH}/configroot/usr/share/ssu
INSTALLS += test_data_etc test_data_usr_share
