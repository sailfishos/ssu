HEADERS = \
        settingstest.h \
        upgradetesthelper.h \

SOURCES = \
        main.cpp \
        settingstest.cpp \
        upgradetesthelper.cpp \

RESOURCES = testdata.qrc
TEMPLATE = app
TARGET = ut_settings
LIBS += -lssu
CONFIG -= app_bundle
CONFIG += console qtestlib
QT -= gui
QT += network testlib

!include( ../tests.pri ) { error("Unable to find tests include") }

unix:target.path = $${PREFIX}/$$TESTS_PATH
INSTALLS += target

!include( ../../buildpath.pri ) { error("Unable to find build path specification") }

update_upgrade_test_recipe.target = update-upgrade-test-recipe
update_upgrade_test_recipe.depends = first
update_upgrade_test_recipe.commands = \
        LD_LIBRARY_PATH="$${LD_LIBRARY_PATH}:${LD_LIBRARY_PATH}" \
                $${DESTDIR}/$${TARGET} -generate-upgrade-test-recipe \
                2>/dev/null > $${PWD}/testdata/upgrade/recipe
update_upgrade_test_recipe.CONFIG += phony
QMAKE_EXTRA_TARGETS += update_upgrade_test_recipe
