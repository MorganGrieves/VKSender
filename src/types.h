#ifndef TYPES_H
#define TYPES_H

#include <QString>

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
    Id id = 0;
    GroupId vkid = "";
    Name name = "";
    Link link = "";
    Link photo = "";
    Members membersCount = 0;
};

QDataStream &operator<<(QDataStream &, const Group &);
QDataStream &operator>>(QDataStream &, Group &);

#endif // TYPES_H
