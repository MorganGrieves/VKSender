#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QPixmap>

#define ORGANIZATION_NAME "Organization Name"
#define APPLICATION_NAME "VKSender"

using Path = QString;

using Id = quint64;
using GroupId = QString;
using Name = QString;
using Link = QString;
using Members = quint64;

using Message = QString;

struct Group
{
    GroupId vkid = "";
    Name name = "";
    Link screenName = "";
    Link photo50Link = "";
    QPixmap photo50;
    bool canPost = false;
};

QDataStream &operator<<(QDataStream &, const Group &);
QDataStream &operator>>(QDataStream &, Group &);

#endif // TYPES_H
