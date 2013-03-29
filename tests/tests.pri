DEPENDPATH *= $${PWD}/sandbox

TESTS_PATH = /opt/tests/ssu
DEFINES += TESTS_PATH="'\"$${TESTS_PATH}\"'"

isEmpty(TARGET):error("TARGET must be defined before this file is included")
TESTS_DATA_PATH = /opt/tests/ssu/data/$${TARGET}
DEFINES += TESTS_DATA_PATH="'\"$${TESTS_DATA_PATH}\"'"
