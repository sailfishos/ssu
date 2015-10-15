/**
 * @file ssufeaturemodel.h
 * @copyright 2015 Jolla Ltd.
 * @author Joona Petrell <joona.petrell@jolla.com>
 * @date 2015
 */

#ifndef _SSUFEATUREMODEL_H
#define _SSUFEATUREMODEL_H

#include <QAbstractListModel>

class SsuFeatureModelPrivate;
class FeatureModelTest;

class SsuFeatureModel: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    enum Roles {
        Name = Qt::UserRole,
        Version
    };

    SsuFeatureModel(QObject *parent = 0);
    SsuFeatureModel(QObject *parent, const QString &path);
    ~SsuFeatureModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

public slots:
    void reload();
    int count() const;

protected:
    QHash<int, QByteArray> roleNames() const;
    const QHash<QByteArray, QString> &featureAt(int index) const;

signals:
    void countChanged();

private:
    SsuFeatureModelPrivate *d;
    friend class FeatureModelTest;
};

#endif
