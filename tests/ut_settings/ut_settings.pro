TARGET = ut_settings
include(../testapplication.pri)
include(ut_settings_dependencies.pri)

HEADERS = \
        settingstest.h \
        upgradetesthelper.h \

SOURCES = \
        main.cpp \
        settingstest.cpp \
        upgradetesthelper.cpp \

RESOURCES = testdata.qrc

update_upgrade_test_recipe.target = update-upgrade-test-recipe
update_upgrade_test_recipe.depends = first
update_upgrade_test_recipe.commands = \
        LD_LIBRARY_PATH="$${DESTDIR_LIB}:${LD_LIBRARY_PATH}" \
                $${DESTDIR}/$${TARGET} -generate-upgrade-test-recipe \
                2>/dev/null > $${PWD}/testdata/upgrade/recipe
update_upgrade_test_recipe.CONFIG += phony
QMAKE_EXTRA_TARGETS += update_upgrade_test_recipe
