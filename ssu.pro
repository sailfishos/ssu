TEMPLATE = subdirs
SUBDIRS = libssu ssud
SUBDIRS += ssucli ssuurlresolver ssuks
SUBDIRS += ssuslipstream declarative

ssuconfhack {
    SUBDIRS += ssuconfperm
}

SUBDIRS += sandboxhook tests tools

ssucli.depends = libssu
ssuurlresolver.depends = libssu
sandboxhook.depends = libssu
tests.depends = libssu sandboxhook
ssuks.depends = libssu
ssud.depends  = libssu
ssuslipstream.depends = libssu
declarative.depends = libssu

config.files = examples/ssu.ini
config.path  = /usr/share/doc/ssu/examples

oneshot.files = ssu-update-repos
oneshot.path = /usr/lib/oneshot.d

macros.files = macros.ssuks
macros.path  = /usr/lib/rpm/macros.d

static_config.files = examples/repos.ini \
                      examples/ssu-defaults.ini \
                      examples/board-mappings.ini
static_config.path  = /usr/share/doc/ssu/examples

INSTALLS += config static_config oneshot macros
