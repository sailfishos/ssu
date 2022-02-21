include(../ssuapplication.pri)
include(tests_common.pri)

QT += testlib

test_data.path = $${TESTS_DATA_PATH}
INSTALLS += test_data

test_data_etc.path = $${TESTS_DATA_PATH}/configroot/etc/ssu
test_data_usr_share.path = $${TESTS_DATA_PATH}/configroot/usr/share/ssu
test_data_usr_share_repos_d.path = $${TESTS_DATA_PATH}/configroot/usr/share/ssu/repos.d
test_data_boardmappings_d.path = $${TESTS_DATA_PATH}/configroot/usr/share/ssu/board-mappings.d

INSTALLS += test_data_etc \
    test_data_usr_share \
    test_data_usr_share_repos_d \
    test_data_boardmappings_d
