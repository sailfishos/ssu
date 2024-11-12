
#include <QDBusConnection>
#include "ssuproxy.h"

SsuProxy::SsuProxy(QObject *parent)
    : SsuDBusInterface("org.nemo.ssu", "/org/nemo/ssu", QDBusConnection::systemBus(), parent)
{
}
