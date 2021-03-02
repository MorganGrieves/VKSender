#include "types.h"

#include <QDataStream>

QDataStream &operator<<(QDataStream &out, const Group &group)
{
    out << group.id;
    out << group.vkid;
    out << group.name;
    out << group.link;
    out << group.photo;
    out << group.membersCount;
    return out;
}

QDataStream &operator>>(QDataStream &in, Group &group)
{
    in >> group.id;
    in >> group.vkid;
    in >> group.name;
    in >> group.link;
    in >> group.photo;
    in >> group.membersCount;
    return in;
}
