#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QPixmap>
#include <QUuid>

#define ORGANIZATION_NAME "Organization Name"
#define APPLICATION_NAME "VKSender"

using Path = QString;

using GroupId = QString;
using Name = QString;
using Link = QString;
using PostNumber = QString;

struct Group
{
    GroupId vkid = "";
    Name name = "";
    Link screenName = "";
    Link photo50Link = "";
    QPixmap photo50;
    bool canPost = false;
};

struct MessagePack
{
    QUuid id;
    QString title;
    QVector<QPair<Group, Qt::CheckState>> groups;
    QVector<Path> photoPaths;
    QVector<Path> videoPaths;
    QVector<Path> filePaths;
    QString message;
};

struct SendingResult
{
    MessagePack message;
    QVector<QPair<Group, PostNumber>> successfulGroups;
    QVector<Group> errorGroups;
};

QDataStream &operator<<(QDataStream &, const MessagePack &);
QDataStream &operator>>(QDataStream &, MessagePack &);

#endif // TYPES_H
