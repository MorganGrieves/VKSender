#include "repository.h"

#include <iostream>
#include <cstdio>
#include <fstream>

#include <QCoreApplication>
#include <QTextStream>
#include <QVariant>
#include <QDir>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonObject>
#include <QJsonParseError>
#include <QtGui>
#include <QtCore>
#include <QtWidgets>

Repository::Repository()
{

}

std::vector<Group> Repository::getGroupData() const
{
    return mGroups;
}

void Repository::clearAll()
{
    mGroups.clear();
}

void Repository::setGroupData(const std::vector<Group> groups)
{
    mGroups = groups;

    emit groupDataUpdated();
}

Repository::~Repository()
{

}
