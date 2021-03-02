#include "repository.h"

#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>

#define ORGANIZATION_NAME "Organization Name"
#define APPLICATION_NAME "VKSender"

Repository::Repository()
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    int size = settings.beginReadArray("mGroups");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        Group group;
        group.id = settings.value("id").toULongLong();
        group.vkid = settings.value("vkid").toString();
        group.name = settings.value("name").toString();
        group.link = settings.value("link").toString();
        group.photo = settings.value("photo").toString();
        group.membersCount = settings.value("membersCount").toULongLong();
        mGroups.append(group);
    }
    settings.endArray();

    emit groupDataUpdated();
}

Repository::~Repository()
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    settings.beginWriteArray("mGroups");
    for (int i = 0; i < mGroups.size(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("id", mGroups.at(i).id);
        settings.setValue("vkid", mGroups.at(i).vkid);
        settings.setValue("name", mGroups.at(i).name);
        settings.setValue("link", mGroups.at(i).link);
        settings.setValue("photo", mGroups.at(i).photo);
        settings.setValue("membersCount", mGroups.at(i).membersCount);
    }
    settings.endArray();
}

void Repository::clearAll()
{
    mGroups.clear();
}

void Repository::serialize(const QString fileName)
{
    QFile *writeFile = new QFile(fileName);
    if (!writeFile->open(QIODevice::WriteOnly))
        qCritical() << "File could not be opened.";

    QDataStream inFile(writeFile);
    inFile.setVersion(QDataStream::Qt_5_15);
    inFile << mGroups;
    writeFile->flush();
    writeFile->close();
    writeFile->deleteLater();
}

void Repository::deserialize(const QString fileName)
{
    mGroups.clear();

    QFile *readFile = new QFile(fileName);
    if (!readFile->open(QFile::ReadOnly))
        qCritical() << "File could not be opened.";

    QDataStream outFile(readFile);
    outFile.setVersion(QDataStream::Qt_5_15);
    outFile >> mGroups;
    readFile->close();
    readFile->deleteLater();

    emit groupDataUpdated();
}

void Repository::setGroupData(const QVector<Group> groups)
{
    mGroups = groups;

    emit groupDataUpdated();
}

QVector<Group> Repository::getGroupData() const
{
    return mGroups;
}
