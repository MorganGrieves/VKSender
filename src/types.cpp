#include "types.h"

#include <QDataStream>

QDataStream &operator<<(QDataStream &out, const Group &group)
{
    out << group.vkid;
    out << group.name;
    out << group.screenName;
    out << group.photo50Link;
    out << group.photo50;
    out << group.canPost;

    return out;
}

QDataStream &operator>>(QDataStream &in, Group &group)
{
    in >> group.vkid;
    in >> group.name;
    in >> group.screenName;
    in >> group.photo50Link;
    in >> group.photo50;
    in >> group.canPost;

    return in;
}
