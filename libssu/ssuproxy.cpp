
#include <QDBusConnection>
#include "ssuproxy.h"

SsuProxy::SsuProxy()
    : SsuDBusInterface("org.nemo.ssu", "/org/nemo/ssu", QDBusConnection::systemBus(), 0)
{
}

