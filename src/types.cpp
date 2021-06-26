#include "types.h"

#include <QDataStream>

QDataStream &operator<<(QDataStream &out, const MessagePack &message)
{
    out << message.version;
    if (message.version == 1)
    {
        out << message.id;
        out << message.title;
        out << message.groups;
        out << message.message;
        out << message.photoPaths;
        out << message.dateCreation;
    }

    return out;
}

QDataStream &operator>>(QDataStream &in, MessagePack &message)
{
    in >> message.version;
    if (message.version == 1)
    {
        in >> message.id;
        in >> message.title;
        in >> message.groups;
        in >> message.message;
        in >> message.photoPaths;
        in >> message.dateCreation;
    }

    return in;
}

QDataStream &operator<<(QDataStream &out, const Group &group)
{
    if (group.version == 1)
    {
        out << group.name;
        out << group.vkid;
        out << group.canPost;
        out << group.photo50;
        out << group.screenName;
        out << group.photo50Link;
    }
    return out;
}

QDataStream &operator>>(QDataStream &in, Group &group)
{
    if (group.version == 1)
    {
        in >> group.name;
        in >> group.vkid;
        in >> group.canPost;
        in >> group.photo50;
        in >> group.screenName;
        in >> group.photo50Link;
    }
    return in;
}
