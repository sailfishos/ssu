TEMPLATE        = subdirs
CONFIG         += ordered coverage debug
SUBDIRS         = \
        testutils \
        testutils/sandboxhook.pro \
        ut_coreconfig \
        ut_deviceinfo \
        ut_repomanager \
        ut_rndssucli \
        ut_sandbox \
        ut_settings \
        ut_ssuurlresolver \
        ut_urlresolver \
        ut_variables \

include(tests_common.pri)
tests.files     = tests.xml
tests.path      = $$TESTS_PATH
INSTALLS += tests
