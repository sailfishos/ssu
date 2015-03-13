contains(QT_VERSION, ^4\\.[0-7]\\..*) {
    error("Can't build with Qt version $${QT_VERSION}. Use at least Qt 4.8.")
}

TEMPLATE = subdirs
SUBDIRS = libssu libssunetworkproxy ssud
SUBDIRS += ssucli ssuurlresolver ssuks ssuslipstream

ssuconfhack {
    SUBDIRS += ssuconfperm
}

SUBDIRS += tests tools

ssucli.depends = libssu
ssuurlresolver.depends = libssu
tests.depends = libssu
ssuks.depends = libssu
ssud.depends  = libssu
ssuslipstream.depends = libssu

config.files = ssu.ini
config.path  = /etc/ssu

oneshot.files = ssu-update-repos
oneshot.path = /usr/lib/oneshot.d

macros.files = macros.ssuks
macros.path  = /etc/rpm/

static_config.files = repos.ini ssu-defaults.ini board-mappings.ini
static_config.path  = /usr/share/ssu

INSTALLS += config static_config oneshot macros

system(qdbusxml2cpp \
    -c SsuAdaptor \
    -a ssud/ssuadaptor.h:ssud/ssuadaptor.cpp \
    dbus/org.nemo.ssu.xml)

system(qdbusxml2cpp \
    -c SsuProxy \
    -p ssucli/ssuproxy.h:ssucli/ssuproxy.cpp \
    dbus/org.nemo.ssu.xml)
