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
    int version = 1;
};

struct MessagePack
{
    QUuid id = "";
    QString title = "";
    QString dateCreation = "";
    QString message = "";
    QVector<QPair<Group, Qt::CheckState>> groups;
    QVector<Path> photoPaths;
    QVector<Path> videoPaths;
    QVector<Path> filePaths;
    int version = 1;
};

struct SendingResult
{
    MessagePack message;
    QVector<QPair<Group, PostNumber>> successfulGroups;
    QVector<Group> errorGroups;
};

struct Version
{
    int major = 0;
    int minor = 1;
    int patch = 0;
};

QDataStream &operator<<(QDataStream &, const MessagePack &);
QDataStream &operator>>(QDataStream &, MessagePack &);

#endif // TYPES_H
