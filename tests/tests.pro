TEMPLATE        = subdirs
CONFIG         += qt ordered coverage debug
SUBDIRS         = \
        sandbox \
        ut_settings \
        ut_ssuurlresolver \
        ut_urlresolver \
        ut_variables \

!include( tests.pri ) { error("Unable to find tests include") }

tests.files     = tests.xml
tests.path      = $$TESTS_PATH
INSTALLS += tests
