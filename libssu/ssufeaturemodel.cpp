/**
 * @file ssufeaturemodel.cpp
 * @copyright 2015 Jolla Ltd.
 * @author Joona Petrell <joona.petrell@jolla.com>
 * @date 2015
 */

#include "ssufeaturemodel.h"
#include <QList>
#include <QSettings>
#include <QHash>
#include <QDebug>
#include <QDirIterator>
#include "../constants.h"

bool featureLessThan(const QHash<QByteArray, QString> &feature1, const QHash<QByteArray, QString> &feature2)
{
    return feature1.value("name") < feature2.value("name");
}

class SsuFeatureModelPrivate {
public:
    SsuFeatureModelPrivate(const QString &p) : path(p)
    {
        load();
    }

    ~SsuFeatureModelPrivate()
    {}

    void load() {
        QDirIterator it(path, QDir::AllEntries|QDir::NoDot|QDir::NoDotDot, QDirIterator::FollowSymlinks);
        QStringList settingsFiles;

        while (it.hasNext()) {
          it.next();
          settingsFiles.append(it.filePath());
        }
        foreach (const QString &settingsFile, settingsFiles) {
            QSettings settings(settingsFile, QSettings::IniFormat);

            foreach (const QString &childGroup, settings.childGroups()) {
                settings.beginGroup(childGroup);
                QStringList keys = settings.childKeys();

                if (keys.contains("name")) {
                    QHash<QByteArray, QString> feature;
                    feature.insert("name", settings.value("name").toString());
                    if (keys.contains("version")) {
                        feature.insert("version", settings.value("version").toString());
                    } else {
                        feature.insert("version", QString(""));
                    }
                    features.append(feature);
                }
                settings.endGroup();
            }
        }
        qSort(features.begin(), features.end(), featureLessThan);
    }

   QString path;
   QList<QHash<QByteArray, QString> > features;
};

SsuFeatureModel::SsuFeatureModel(QObject *parent, const QString &path)
    : QAbstractListModel(parent),
      d(new SsuFeatureModelPrivate(path))
{
}

SsuFeatureModel::SsuFeatureModel(QObject *parent)
    : QAbstractListModel(parent),
      d(new SsuFeatureModelPrivate(SSU_FEATURE_CONFIGURATION_DIR))
{
}

SsuFeatureModel::~SsuFeatureModel()
{
    delete d;
}

int SsuFeatureModel::count() const
{
    return rowCount();
}

const QHash<QByteArray, QString> &SsuFeatureModel::featureAt(int index) const
{
    return d->features.at(index);
}

int SsuFeatureModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d->features.count();
}

void SsuFeatureModel::reload()
{
    beginResetModel();
    d->features.clear();
    d->load();
    endResetModel();
}

QVariant SsuFeatureModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (!index.isValid() || row < 0 || row >= d->features.count())
        return QVariant();

    switch (role) {
    case Name:
        return d->features.at(row).value("name");
    case Version:
        return d->features.at(row).value("version");
    }
    return QVariant();
}

QHash<int, QByteArray> SsuFeatureModel::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles.insert(Name, "name");
    roles.insert(Version, "version");
    return roles;
}
