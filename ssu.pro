contains(QT_VERSION, ^4\\.[0-7]\\..*) {
    error("Can't build with Qt version $${QT_VERSION}. Use at least Qt 4.8.")
}

TEMPLATE = subdirs
SUBDIRS = libssu
SUBDIRS += rndssucli rndregisterui ssuurlresolver

ssuconfhack {
    SUBDIRS += ssuconfperm
}

SUBDIRS += tests tools

rndssucli.depends = libssu
rndregisterui.depends = libssu
ssuurlresolver.depends = libssu
tests.depends = libssu

config.files = ssu.ini
config.path  = /etc/ssu

static_config.files = repos.ini ssu-defaults.ini board-mappings.ini
static_config.path  = /usr/share/ssu

INSTALLS += config static_config
