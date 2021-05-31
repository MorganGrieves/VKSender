#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <QObject>
#include <QString>
#include <QVector>

#include "types.h"

class Repository : public QObject
{
    Q_OBJECT

public:
    Repository();
    ~Repository();
    void clearAll();

    void serialize(const QString fileName);
    void deserialize(const QString fileName);

    void setGroupData(const QVector<Group> groups);
    QVector<Group> getGroupData() const;

signals:
    void groupDataUpdated();

private:
    QVector<Group> mGroups;

    QVector<QPair<Message, QVector<Group>>> mSendingVector;
};

#endif // REPOSITORY_H
