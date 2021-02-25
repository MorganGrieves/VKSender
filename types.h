#ifndef TYPES_H
#define TYPES_H
#include <QString>
using Path = QString;

using Id = std::size_t;
using GroupId = QString;
using Name = QString;
using Link = QString;
using Members = unsigned int;

struct Group
{
    Id id = 0;
    GroupId vkid = "";
    Name name = "";
    Link link = "";
    Link photo = "";
    Members membersCount = 0;
};

#endif // TYPES_H
