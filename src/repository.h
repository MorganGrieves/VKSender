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

    void serialize(const QVector<MessagePack> messages, const QString fileName);
    QVector<MessagePack> deserialize(const QString fileName);
};

#endif // REPOSITORY_H
