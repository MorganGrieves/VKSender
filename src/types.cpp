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
