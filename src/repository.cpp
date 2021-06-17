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
//    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
//    int size = settings.beginReadArray("mGroups");
//    for (int i = 0; i < size; ++i)
//    {
//        settings.setArrayIndex(i);
//        Group group;
//        group.id = settings.value("id").toULongLong();
//        group.vkid = settings.value("vkid").toString();
//        group.name = settings.value("name").toString();
//        group.link = settings.value("link").toString();
//        group.photo = settings.value("photo").toString();
//        group.membersCount = settings.value("membersCount").toULongLong();
//        mGroups.append(group);
//    }
//    settings.endArray();

//    emit groupDataUpdated();
}

Repository::~Repository()
{
//    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
//    settings.beginWriteArray("mGroups");
//    for (int i = 0; i < mGroups.size(); ++i)
//    {
//        settings.setArrayIndex(i);
//        settings.setValue("id", mGroups.at(i).id);
//        settings.setValue("vkid", mGroups.at(i).vkid);
//        settings.setValue("name", mGroups.at(i).name);
//        settings.setValue("link", mGroups.at(i).link);
//        settings.setValue("photo", mGroups.at(i).photo);
//        settings.setValue("membersCount", mGroups.at(i).membersCount);
//    }
//    settings.endArray();
}

void Repository::serialize(const QVector<MessagePack> messages, const QString fileName)
{
    QFile *writeFile = new QFile(fileName);
    if (!writeFile->open(QIODevice::WriteOnly))
        qCritical() << "File could not be opened.";
    qDebug() << "messages were serialized";

    QDataStream inFile(writeFile);
    inFile.setVersion(QDataStream::Qt_5_15);
    inFile << messages;
    writeFile->flush();
    writeFile->close();
    writeFile->deleteLater();
}

QVector<MessagePack> Repository::deserialize(const QString fileName)
{
    QFile *readFile = new QFile(fileName);
    if (!readFile->open(QFile::ReadOnly))
        qCritical() << "File could not be opened.";

    QVector<MessagePack> result;
    QDataStream outFile(readFile);
    outFile.setVersion(QDataStream::Qt_5_15);
    outFile >> result;
    readFile->close();
    readFile->deleteLater();
    qDebug() << "messages were deserialized";

    return result;
}
