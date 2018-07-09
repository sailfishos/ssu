#ifndef SSUD_INCLUDE_H
#define SSUD_INCLUDE_H

#include <QDBusArgument>
#include <QString>
#include <QVariantMap>

struct SsuRepo {
    QString name;
    QString url;
    QVariantMap parameters;
};

inline QDBusArgument &operator<<(QDBusArgument &argument, const SsuRepo &myRepo)
{
    argument.beginStructure();
    argument << myRepo.name << myRepo.url << myRepo.parameters;
    argument.endStructure();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, SsuRepo &myRepo)
{
    argument.beginStructure();
    argument >> myRepo.name >> myRepo.url >> myRepo.parameters;
    argument.endStructure();
    return argument;
}

Q_DECLARE_METATYPE(SsuRepo)

#endif // SSUD_INCLUDE_H
