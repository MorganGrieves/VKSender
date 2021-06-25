#include "types.h"

#include <QDataStream>

QDataStream &operator<<(QDataStream &out, const MessagePack &message)
{
    out << message.id;
    out << message.title;
    out << message.groups;
    out << message.message;
    out << message.photoPaths;

    return out;
}

QDataStream &operator>>(QDataStream &in, MessagePack &message)
{
    in >> message.id;
    in >> message.title;
    in >> message.groups;
    in >> message.message;
    in >> message.photoPaths;

    return in;
}

QDataStream &operator<<(QDataStream &out, const Group &group)
{
    out << group.name;
    out << group.vkid;
    out << group.canPost;
    out << group.photo50;
    out << group.screenName;
    out << group.photo50Link;

    return out;
}

QDataStream &operator>>(QDataStream &in, Group &group)
{
    in >> group.name;
    in >> group.vkid;
    in >> group.canPost;
    in >> group.photo50;
    in >> group.screenName;
    in >> group.photo50Link;

    return in;
}
