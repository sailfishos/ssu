TEMPLATE        = subdirs
CONFIG         += ordered coverage debug
SUBDIRS         = \
        testutils \
        formatoutput \
        testutils/sandboxhook.pro \
        ut_coreconfig \
        ut_deviceinfo \
        ut_featuremodel \
        ut_repomanager \
        ut_ssu \
        ut_ssucli \
        ut_sandbox \
        ut_settings \
        ut_ssuurlresolver \
        ut_urlresolver \
        ut_variables \

include(tests_common.pri)
tests.files     = tests.xml
tests.path      = $$TESTS_PATH

scripts.files   = ../run-tests
scripts.path    = $$TESTS_PATH

INSTALLS += tests scripts
