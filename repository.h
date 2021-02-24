#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <vector>
#include <fstream>
#include <iostream>

#include <QObject>
#include <QDateTime>
#include <QFile>
#include <string>
#include <QDataStream>
#include <QString>
#include <QNetworkReply>
#include <QCoreApplication>
#include <QDir>

#include "types.h"

class Repository : public QObject
{
    Q_OBJECT

public:
    Repository();
    ~Repository();
    void clearAll();

    void setGroupData(const std::vector<Group> groups);
    std::vector<Group> getGroupData() const;

signals:
    void needGroupData();
    void groupDataUpdated();

private:
    std::vector<Group> mGroups;
};

#endif // REPOSITORY_H
