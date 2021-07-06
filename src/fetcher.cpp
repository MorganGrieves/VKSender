#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonObject>
#include <QtGui>
#include <QtCore>
#include <QtWidgets>
#include <QEventLoop>
#include <QTimer>
#include <QHttpPart>
#include <QHttpMultiPart>
#include <QMessageBox>
#include <QByteArray>

#include "fetcher.h"

static Request vkApi;

Fetcher::Fetcher(QObject *parent) : QObject(parent)
{
    mNetworkManager = new QNetworkAccessManager(this);


//    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
//    if(!settings.value("Fetcher/vkApi.implicitFlowAccessToken").toString().isEmpty())
//        setAccessToken(settings.value("Fetcher/vkApi.implicitFlowAccessToken").toString());

}

Fetcher::~Fetcher()
{
//    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
//    settings.setValue("Fetcher/vkApi.implicitFlowAccessToken", vkApi.implicitFlowAccessToken);
}

void Fetcher::setRepository(const std::shared_ptr<Repository> repository)
{
    mRepository = repository;
}

bool Fetcher::tokenIsEmpty() const
{
    return (vkApi.implicitFlowAccessToken.isEmpty());
}

void Fetcher::setAccessToken(const QString &token)
{
    vkApi.implicitFlowAccessToken = token;
    qDebug() << "Token was set";
    onUserDataUpdate();
}

void Fetcher::getGroupInfoById(const QString &id)
{
    QUrl requestUrl(mVkApiLink
                    + vkApi.groupById.method);
    QUrlQuery params;
    params.addQueryItem("access_token", vkApi.implicitFlowAccessToken);
    params.addQueryItem("v", vkApi.apiVersion);
    params.addQueryItem("fields", vkApi.groupById.fields);
    params.addQueryItem("group_id", id);
    requestUrl.setQuery(params.query());

    QNetworkRequest request;
    request.setUrl(requestUrl);

    QNetworkReply *reply = mNetworkManager->get(request);

    connect(reply, &QNetworkReply::finished,
            [reply, this]()
    {
        QJsonParseError parseError;
        const auto data = reply->readAll();
        const auto document = QJsonDocument::fromJson(data, &parseError);

        if (isJsonErrorReturned(parseError)
                || isServerErrorReturned(document)
                || isReplyErrorReturned(*reply))
        {
            qDebug() << "error vkApi.groupById.groupId";
            reply->deleteLater();
            return;
        }

        const QJsonObject jsonResponse = document.object()["response"].toArray().at(0).toObject();
        qDebug() << jsonResponse;

        Group group;

        group.vkid = QString::number(jsonResponse["id"].toDouble(), 'f', 0);
        group.name = jsonResponse["name"].toString();
        group.screenName = jsonResponse["screen_name"].toString();
        group.photo50Link = jsonResponse["photo_50"].toString();
        group.photo50 = *downloadPhoto(QUrl(group.photo50Link));
        group.canPost = jsonResponse["can_post"].toBool();
        group.type = jsonResponse["type"].toString();

        emit onGroupUpdated(group);
        reply->deleteLater();
    });
}

const QPixmap &Fetcher::getUserPhoto100() const
{
    return mUserInfo.userPhoto100;
}

const QString &Fetcher::getUserName() const
{
    return mUserInfo.userName;
}

const QVector<Group> &Fetcher::getUserGroups() const
{
    return mUserInfo.userGroups;
}

void Fetcher::sendMessage(const MessagePack &pack)
{
    QString onceAddingAttachment = "";
    try
    {
        onceAddingAttachment += uploadVideosToGroup(pack.videoPaths, pack.id);
        onceAddingAttachment += uploadAudiosToGroup(pack.audioPaths, pack.id);
    }
    catch (...)
    {
        return;
    }

    for (const auto& group : pack.groups)
    {
        if (group.second == Qt::Unchecked)
            continue;

        try
        {
            QString attachments = "";

            attachments += onceAddingAttachment;

            attachments += uploadPhotosToGroup(group.first, pack.photoPaths, pack.id);

            if (group.first.type != "page")
                attachments += uploadDocsToGroup(group.first, pack.docPaths, pack.id);

            uploadMessageToGroup(pack.id, group.first, pack.message, attachments);
        }
        catch (...)
        {
            continue;
        }
    }

    emit sendingFinished(pack.id);
}

void Fetcher::onPostDelete(const QString postId, const QString ownerId)
{
    QUrl wallDeleteRequestUrl(mVkApiLink
                    + vkApi.wallDelete.method
                    );

    QUrlQuery params;
    params.addQueryItem("access_token", vkApi.implicitFlowAccessToken);
    params.addQueryItem("v", vkApi.apiVersion);
    params.addQueryItem("owner_id", "-" + ownerId);
    params.addQueryItem("post_id", postId);
    wallDeleteRequestUrl.setQuery(params);

    QNetworkRequest request(wallDeleteRequestUrl);

    QNetworkReply *reply = mNetworkManager->get(request);

    connect(reply, &QNetworkReply::finished,
            [reply, ownerId, postId, this]()
    {
        QString response;
        QJsonParseError parseError;
        const auto data = reply->readAll();
        const auto document = QJsonDocument::fromJson(data, &parseError);

        if (isJsonErrorReturned(parseError)
                || isServerErrorReturned(document)
                || isReplyErrorReturned(*reply))
        {
            qCritical() << "reply vkApi.wallDelete error finished: "
                       << postId << ownerId;
            reply->deleteLater();
            return;
        }

        int responseNumber = document.object()["response"].toInt();
        const int SUCCESS_NUMBER = 1;
        if (responseNumber == SUCCESS_NUMBER)
        {
            reply->deleteLater();
            emit deletedPost(postId, ownerId);
        }
        else
        {
            qCritical() << "error vkApi.wallDelete error finished:"
                        << postId << ownerId << document.object()["response"];
            reply->deleteLater();
            return;
        }
    });
}

void Fetcher::onUserDataUpdate()
{
    downloadUserInfo();
    downloadUserGroups();
}

bool Fetcher::isReplyErrorReturned(const QNetworkReply &reply) const
{
    if (reply.error())
    {
        qCritical() << reply.errorString();
        return true;
    }

    return false;
}

bool Fetcher::isJsonErrorReturned(const QJsonParseError &error) const
{
    if (error.error)
    {
        qCritical() << "jsonErrorReturned" << error.errorString();
        return true;
    }

    return false;
}

bool Fetcher::isServerErrorReturned(const QJsonDocument &document) const
{
    const QJsonObject jsonServerErrorObject = document.object()["error"].toObject();

    if (!jsonServerErrorObject.isEmpty())
    {
        qCritical() << document;
        return true;
    }

    return false;
}

void Fetcher::setUserPhoto100(const QPixmap &userPhoto)
{
    mUserInfo.userPhoto100 = userPhoto;
    emit userPhoto100Update();
}

void Fetcher::setUserName(const QString &userName)
{
    mUserInfo.userName = userName;
    emit userNameUpdate();
}

void Fetcher::setUserId(const int &id)
{
    mUserInfo.userId = id;
}

void Fetcher::setUserPhoto100Link(const QString &link)
{
    mUserInfo.userPhoto100Link = link;
}

void Fetcher::setUserGroups(const QVector<Group> groups)
{
    mUserInfo.userGroups = groups;
    emit userGroupsUpdate();
}

void Fetcher::downloadUserInfo()
{
    QUrl photo100RequestUrl(mVkApiLink
                            + vkApi.usersGet.method
                            );
    QUrlQuery params;
    params.addQueryItem("access_token", vkApi.implicitFlowAccessToken);
    params.addQueryItem("v", vkApi.apiVersion);
    params.addQueryItem("fields", "photo_100");
    params.addQueryItem("name_case", "Nom");
    photo100RequestUrl.setQuery(params);

    QNetworkRequest request(photo100RequestUrl);

    QNetworkReply *reply = mNetworkManager->get(request);

    connect(reply, &QNetworkReply::finished,
            [reply, this]
    {
        QString response;
        QJsonParseError parseError;
        const auto data = reply->readAll();
        const auto document = QJsonDocument::fromJson(data, &parseError);

        if (isJsonErrorReturned(parseError)
                || isServerErrorReturned(document)
                || isReplyErrorReturned(*reply))
        {
            qCritical() << "error vkApi.usersGet finished:";
            reply->deleteLater();
            return;
        }

        const QJsonArray jsonResponse = document.object()["response"].toArray();
        qDebug() << "vk.usersGet" << jsonResponse;

        setUserName(jsonResponse.at(0).toObject()["last_name"].toString() + " "
                + jsonResponse.at(0).toObject()["first_name"].toString());
        setUserId(jsonResponse.at(0).toObject()["id"].toInt());
        setUserPhoto100(*downloadPhoto(QUrl(jsonResponse.at(0).toObject()["photo_100"].toString())));
        setUserPhoto100Link(jsonResponse.at(0).toObject()["photo_100"].toString());
    });
}

void Fetcher::downloadUserGroups()
{
    QUrl groupsRequestUrl(mVkApiLink
                            + vkApi.groupsGet.method
                            );
    QUrlQuery params;
    params.addQueryItem("access_token", vkApi.implicitFlowAccessToken);
    params.addQueryItem("v", vkApi.apiVersion);
    params.addQueryItem("fields", vkApi.groupsGet.fields);
    params.addQueryItem("filter", vkApi.groupsGet.filter);
    params.addQueryItem("extended", vkApi.groupsGet.extended);
    params.addQueryItem("count", vkApi.groupsGet.count);
    groupsRequestUrl.setQuery(params);

    QNetworkRequest request(groupsRequestUrl);

    QNetworkReply *reply = mNetworkManager->get(request);

    connect(reply, &QNetworkReply::finished,
            [reply, this]
    {
        QString response;
        QJsonParseError parseError;
        const auto data = reply->readAll();
        const auto document = QJsonDocument::fromJson(data, &parseError);

        if (isJsonErrorReturned(parseError)
                || isServerErrorReturned(document)
                || isReplyErrorReturned(*reply))
        {
            qCritical() << "error vkApi.groupsGet finished:";
            reply->deleteLater();
            return;
        }

        const QJsonObject jsonResponse = document.object()["response"].toObject();
        qDebug() << "vk.groupsGet" << jsonResponse;

        QVector<Group> groups;
        const QJsonArray groupJsonArray = jsonResponse["items"].toArray();

        foreach (const QJsonValue &value, groupJsonArray)
        {
            QJsonObject groupInfo = value.toObject();

            Group group;
            group.vkid = QString::number(groupInfo["id"].toDouble(), 'f', 0);
            group.name = groupInfo["name"].toString();
            group.screenName = groupInfo["screen_name"].toString();
            group.photo50Link = groupInfo["photo_50"].toString();
            group.photo50 = *downloadPhoto(QUrl(group.photo50Link));
            group.canPost = groupInfo["can_post"].toInt() ? true : false;
            group.type = groupInfo["type"].toString();

            if (!group.canPost)
                continue;

            groups.push_back(group);
        }

        setUserGroups(groups);
        reply->deleteLater();
    });
}

QPixmap *Fetcher::downloadPhoto(const QUrl &requestUrl) const
{
    QNetworkRequest request(requestUrl);
    QNetworkReply *reply = mNetworkManager->get(request);
    QPixmap *result = new QPixmap();
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished,
            &loop, &QEventLoop::quit);

    connect(reply, &QNetworkReply::finished,
            [reply, this, result]
    {
        if (isReplyErrorReturned(*reply))
        {
            qCritical() << "error requestUrl finished:";
            reply->deleteLater();
            return;
        }

        result->loadFromData(reply->readAll());
        reply->deleteLater();
    });
    loop.exec();

    return result;
}

void Fetcher::uploadMessageToGroup(const QUuid &id, const Group &group, const QString &message, const QString &attachments)
{
    QEventLoop groupLoop;
    connect(this, &Fetcher::sentMessage, &groupLoop, &QEventLoop::quit);

    QUrl requestUrl(mVkApiLink
                    + vkApi.wallPost.method + "?"
                     );
    QUrlQuery params;
    params.addQueryItem("access_token", vkApi.implicitFlowAccessToken);
    params.addQueryItem("v", vkApi.apiVersion);
    params.addQueryItem("message", message);
    params.addQueryItem("owner_id", "-" + group.vkid);
    params.addQueryItem("attachments", attachments);
    requestUrl.setQuery(params.query());

    QNetworkRequest request;
    request.setUrl(requestUrl);

    QNetworkReply *wallPostReply = mNetworkManager->get(request);

    connect(wallPostReply, &QNetworkReply::finished,
            [wallPostReply,
            group,
            message,
            attachments,
            id,
            &groupLoop,
            this]()
    {
        QJsonParseError parseError;
        const auto data = wallPostReply->readAll();
        const auto document = QJsonDocument::fromJson(data, &parseError);

        if (isJsonErrorReturned(parseError)
                || isServerErrorReturned(document)
                || isReplyErrorReturned(*wallPostReply))
        {
            qCritical() << "error vkApi.photos.saveWallPhoto finished:"
                        << group.name << group.vkid << "|";
            wallPostReply->deleteLater();
            groupLoop.quit();
            throw "vkApi.photos.saveWallPhoto";
        }

        QString postIdInfo = QString::number(document.object().value("response").toObject()["post_id"].toInt());

        qDebug() << "vkApi.photos.getWallUploadServer" << postIdInfo;

        emit sentMessage(id, group, postIdInfo);
        wallPostReply->deleteLater();
    });

    groupLoop.exec();
}

QString Fetcher::uploadPhotosToGroup(const Group &group, const QVector<Path> &photoPaths, QUuid id)
{
    QString attachments = "";

    if (photoPaths.empty()) return attachments;

    int sendedPhoto = 0;
    QEventLoop loop;
    QObject *obj = new QObject(this);
    connect(this, &Fetcher::updatedPhoto, obj,
            [&sendedPhoto,
            photoPaths,
            &loop,
            obj](QUuid id)
    {
        if (++sendedPhoto == photoPaths.size())
        {
            obj->deleteLater();
            loop.quit();
        }

    });

    QUrl getWallUpdateRequestUrl(mVkApiLink
                    + vkApi.photos.getWallUploadServer.method
                    );
    QUrlQuery params;
    params.addQueryItem("access_token", vkApi.implicitFlowAccessToken);
    params.addQueryItem("v", vkApi.apiVersion);
    params.addQueryItem("group_id", group.vkid);
    getWallUpdateRequestUrl.setQuery(params);

    QNetworkRequest getWallUpdateRequest;
    getWallUpdateRequest.setUrl(getWallUpdateRequestUrl);
    getWallUpdateRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                                   "application/x-www-form-urlencoded");

    QNetworkReply *reply = mNetworkManager->post(getWallUpdateRequest, params.query().toUtf8());

    connect(reply, &QNetworkReply::finished,
            [reply,
            photoPaths,
            group,
            &loop,
            &attachments,
            id,
            this]()
    {
        QString uploadUrl;
        QJsonParseError parseError;
        const auto data = reply->readAll();
        const auto document = QJsonDocument::fromJson(data, &parseError);

        if (isJsonErrorReturned(parseError)
                || isServerErrorReturned(document)
                || isReplyErrorReturned(*reply))
        {
            qCritical() << "error  vkApi.photos.getWallUploadServer finished:"
                        << parseError.errorString() << " | " << group.name << group.vkid << "|";
            reply->deleteLater();
            loop.quit();
            throw "vkApi.photos.getWallUploadServer";
        }

        QJsonObject serverInfo = document.object().value("response").toObject();

        qDebug() << "vkApi.photos.getWallUploadServer" << serverInfo;

        uploadUrl = serverInfo["upload_url"].toString();

        for (std::size_t i = 0; i < photoPaths.size(); i++)
        {
            QTimer::singleShot(500,
                               [reply,
                               photoPaths,
                               group,
                               &attachments,
                               uploadUrl,
                               &loop,
                               i,
                               id,
                               this]()
            {
                QFile *file = new QFile(photoPaths.at(i));

                if (!file->open(QIODevice::ReadOnly))
                {
                    qCritical() << "File could not be opened:" << photoPaths.at(i);
                    reply->deleteLater();
                    loop.quit();
                    throw "File could not be opened";
                }

                QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

                QHttpPart imagePart;
                imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/gif, image/jpg, image/jpeg, image/png"));
                imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; "
                                                                                        "name=\"photo\"; "
                                                                                        "filename=\"" + file->fileName() + "\""));
                imagePart.setBodyDevice(file);
                file->setParent(multiPart);

                multiPart->append(imagePart);

                QNetworkRequest uploadUrlRequest;
                uploadUrlRequest.setUrl(QUrl(uploadUrl));
                QNetworkReply *transferReply = mNetworkManager->post(uploadUrlRequest, multiPart);

                multiPart->setParent(transferReply);
                connect(transferReply, &QNetworkReply::finished,
                        [transferReply,
                        group,
                        id,
                        &attachments,
                        &loop,
                        this]()
                {
                    QTimer::singleShot(500,
                                       [transferReply,
                                       group,
                                       &attachments,
                                       &loop,
                                       id,
                                       this]()
                    {
                        QJsonParseError parseError;
                        const auto data = transferReply->readAll();
                        const auto document = QJsonDocument::fromJson(data, &parseError);

                        const QJsonObject jsonServerErrorObject = document.object()["error"].toObject();

                        if (isJsonErrorReturned(parseError)
                                || isServerErrorReturned(document)
                                || isReplyErrorReturned(*transferReply))
                        {
                            qCritical() << "error uploadUrlRequest finished:"
                                        << parseError.errorString() << " | " << group.name << group.vkid << "|";
                            transferReply->deleteLater();
                            loop.quit();
                            throw "uploadUrlRequest";
                        }

                        const QJsonObject jsonResponse = document.object();

                        qDebug() << "vkApi.photos.saveWallPhoto" << jsonResponse;

                        QUrl requestUrl(mVkApiLink
                                        + vkApi.photos.saveWallPhoto.method + "?"
                                        );

                        QUrlQuery params;
                        params.addQueryItem("access_token",vkApi.implicitFlowAccessToken);
                        params.addQueryItem("v", vkApi.apiVersion);
                        params.addQueryItem("group_id", group.vkid);
                        params.addQueryItem("hash", jsonResponse["hash"].toString());
                        params.addQueryItem("server", QString::number(jsonResponse["server"].toInt()));
                        params.addQueryItem("photo", jsonResponse["photo"].toString().toUtf8());

                        QNetworkRequest request;
                        request.setUrl(requestUrl);
                        request.setHeader(QNetworkRequest::ContentTypeHeader,
                                          "application/x-www-form-urlencoded");

                        QNetworkReply *newReply = mNetworkManager->post(request, params.query().toUtf8());

                        connect(newReply, &QNetworkReply::finished,
                                [newReply,
                                id,
                                group,
                                &attachments,
                                &loop,
                                this]()
                        {
                            QJsonParseError parseError;
                            const auto data = newReply->readAll();
                            const auto document = QJsonDocument::fromJson(data, &parseError);

                            if (isJsonErrorReturned(parseError)
                                    || isServerErrorReturned(document)
                                    || isReplyErrorReturned(*newReply))
                            {
                                qCritical() << "error vkApi.photos.saveWallPhoto finished: "
                                            << group.name << group.vkid << "|";
                                newReply->deleteLater();
                                loop.quit();
                                throw "vkApi.photos.saveWallPhoto";
                            }

                            const QJsonObject jsonResponse = document.object()["response"].toArray()[0].toObject();
                            qDebug() << "saveWallPhoto" << jsonResponse;

                            attachments += "photo" + QString::number(jsonResponse["owner_id"].toInt()) +
                                    "_" + QString::number(jsonResponse["id"].toInt()) + ",";

                            emit updatedPhoto(id);

                            newReply->deleteLater();
                        });

                        transferReply->deleteLater();
                    });
                });

            });

        }

        reply->deleteLater();
    });
   loop.exec();

   return attachments;
}

QString Fetcher::uploadAudiosToGroup(const QVector<Path> &audioPaths, QUuid id)
{
    QString attachments = "";

    if (audioPaths.empty()) return attachments;

    int sendedAudios = 0;

    QEventLoop loop;
    QObject *obj = new QObject(this);
    connect(this, &Fetcher::updatedAudio, obj,
            [&sendedAudios,
            audioPaths,
            &loop,
            obj](QUuid id)
    {
        if (++sendedAudios == audioPaths.size())
        {
            obj->deleteLater();
            loop.quit();
        }
    });
    QUrl getUpdateRequestUrl(mVkApiLink
                    + vkApi.audios.getUploadServer.method
                    );
    QUrlQuery params;
    params.addQueryItem("access_token", vkApi.implicitFlowAccessToken);
    params.addQueryItem("v", vkApi.apiVersion);
    getUpdateRequestUrl.setQuery(params);

    QNetworkRequest getUpdateRequest;
    getUpdateRequest.setUrl(getUpdateRequestUrl);
    getUpdateRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                                   "application/x-www-form-urlencoded");

    QNetworkReply *reply = mNetworkManager->post(getUpdateRequest, params.query().toUtf8());

    connect(reply, &QNetworkReply::finished,
            [reply,
            audioPaths,
            &loop,
            &attachments,
            id,
            this]()
    {
        QString uploadUrl;
        QJsonParseError parseError;
        const auto data = reply->readAll();
        const auto document = QJsonDocument::fromJson(data, &parseError);

        if (isJsonErrorReturned(parseError)
                || isServerErrorReturned(document)
                || isReplyErrorReturned(*reply))
        {
            qCritical() << "error  vkApi.audios.getUploadServer finished:"
                        << parseError.errorString() << "|";
            reply->deleteLater();
            loop.quit();
            throw "vkApi.audios.getUploadServer";
        }

        QJsonObject serverInfo = document.object().value("response").toObject();

        qDebug() << "vkApi.audios.getUploadServer" << serverInfo;

        uploadUrl = serverInfo["upload_url"].toString();

        for (std::size_t i = 0; i < audioPaths.size(); i++)
        {
            QTimer::singleShot(500,
                               [reply,
                               audioPaths,
                               &attachments,
                               uploadUrl,
                               &loop,
                               i,
                               id,
                               this]()
            {
                QFile *file = new QFile(audioPaths.at(i));

                if (!file->open(QIODevice::ReadOnly))
                {
                    qCritical() << "File could not be opened:" << audioPaths.at(i);
                    reply->deleteLater();
                    loop.quit();
                    throw "File could not be opened";
                }

                QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

                QHttpPart imagePart;
                imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("audio/mpeg"));
                imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; "
                                                                                        "name=\"file\"; "
                                                                                        "filename=\"" + file->fileName() + "\""));
                imagePart.setBodyDevice(file);
                file->setParent(multiPart);

                multiPart->append(imagePart);

                QNetworkRequest uploadUrlRequest;
                uploadUrlRequest.setUrl(QUrl(uploadUrl));
                QNetworkReply *transferReply = mNetworkManager->post(uploadUrlRequest, multiPart);

                multiPart->setParent(transferReply);
                connect(transferReply, &QNetworkReply::finished,
                        [transferReply,
                        id,
                        &attachments,
                        &loop,
                        this]()
                {
                    QTimer::singleShot(500,
                                       [transferReply,
                                       &attachments,
                                       &loop,
                                       id,
                                       this]()
                    {
                        QJsonParseError parseError;
                        const auto data = transferReply->readAll();
                        const auto document = QJsonDocument::fromJson(data, &parseError);

                        const QJsonObject jsonServerErrorObject = document.object()["error"].toObject();

                        if (isJsonErrorReturned(parseError)
                                || isServerErrorReturned(document)
                                || isReplyErrorReturned(*transferReply))
                        {
                            qCritical() << "error uploadUrlRequest finished:"
                                        << parseError.errorString() << "|";
                            transferReply->deleteLater();
                            loop.quit();
                            throw "uploadUrlRequest";
                        }

                        const QJsonObject jsonResponse = document.object();

                        qDebug() << "vkApi.audios.save" << jsonResponse;

                        QUrl requestUrl(mVkApiLink
                                        + vkApi.audios.save.method + "?"
                                        );

                        QUrlQuery params;
                        params.addQueryItem("access_token",vkApi.implicitFlowAccessToken);
                        params.addQueryItem("v", vkApi.apiVersion);
                        params.addQueryItem("redirect", jsonResponse["redirect"].toString());
                        params.addQueryItem("hash", jsonResponse["hash"].toString());
                        params.addQueryItem("server", QString::number(jsonResponse["server"].toInt()));
                        params.addQueryItem("audio", jsonResponse["audio"].toString());

                        QNetworkRequest request;
                        request.setUrl(requestUrl);
                        request.setHeader(QNetworkRequest::ContentTypeHeader,
                                          "application/x-www-form-urlencoded");

                        QNetworkReply *newReply = mNetworkManager->post(request, params.query().toUtf8());

                        connect(newReply, &QNetworkReply::finished,
                                [newReply,
                                id,
                                &attachments,
                                &loop,
                                this]()
                        {
                            QJsonParseError parseError;
                            const auto data = newReply->readAll();
                            const auto document = QJsonDocument::fromJson(data, &parseError);

                            if (isJsonErrorReturned(parseError)
                                    || isServerErrorReturned(document)
                                    || isReplyErrorReturned(*newReply))
                            {
                                qCritical() << "error vkApi.audios.save finished: " << "|";
                                newReply->deleteLater();
                                loop.quit();
                                throw "vkApi.audios.save";
                            }

                            const QJsonObject jsonResponse = document.object()["response"].toObject();
                            qDebug() << "save" << jsonResponse;

                            attachments += "audio" + QString::number(jsonResponse["owner_id"].toInt()) +
                                    "_" + QString::number(jsonResponse["id"].toInt()) + ",";

                            emit updatedAudio(id);

                            newReply->deleteLater();
                        });

                        transferReply->deleteLater();
                    });
                });

            });
        }

        reply->deleteLater();
    });
   loop.exec();

   return attachments;
}

QString Fetcher::uploadDocsToGroup(const Group &group, const QVector<Path> &docsPaths, QUuid id)
{
    QString attachments = "";

    if (docsPaths.empty()) return attachments;

    int sendedDoc = 0;
    QEventLoop loop;
    QObject *obj = new QObject(this);
    connect(this, &Fetcher::updatedDoc, obj,
            [&sendedDoc,
            docsPaths,
            &loop,
            obj](QUuid id)
    {
        if (++sendedDoc == docsPaths.size())
        {
            obj->deleteLater();
            loop.quit();
        }
    });

    QUrl getWallUpdateRequestUrl(mVkApiLink
                    + vkApi.docs.getWallUploadServer.method
                    );
    QUrlQuery params;
    params.addQueryItem("access_token", vkApi.implicitFlowAccessToken);
    params.addQueryItem("v", vkApi.apiVersion);
    params.addQueryItem("group_id", group.vkid);
    getWallUpdateRequestUrl.setQuery(params);

    QNetworkRequest getWallUpdateRequest;
    getWallUpdateRequest.setUrl(getWallUpdateRequestUrl);
    getWallUpdateRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                                   "application/x-www-form-urlencoded");

    QNetworkReply *reply = mNetworkManager->post(getWallUpdateRequest, params.query().toUtf8());

    connect(reply, &QNetworkReply::finished,
            [reply,
            docsPaths,
            group,
            &loop,
            &attachments,
            id,
            this]()
    {
        QString uploadUrl;
        QJsonParseError parseError;
        const auto data = reply->readAll();
        const auto document = QJsonDocument::fromJson(data, &parseError);

        if (isJsonErrorReturned(parseError)
                || isServerErrorReturned(document)
                || isReplyErrorReturned(*reply))
        {
            qCritical() << "error  vkApi.docs.getWallUploadServer finished:"
                        << parseError.errorString() << " | " << group.name << group.vkid << "|";
            reply->deleteLater();
            loop.quit();
            throw "vkApi.docs.getWallUploadServer";
        }

        QJsonObject serverInfo = document.object().value("response").toObject();

        qDebug() << "vkApi.docs.getWallUploadServer" << serverInfo;

        uploadUrl = serverInfo["upload_url"].toString();

        for (std::size_t i = 0; i < docsPaths.size(); i++)
        {
            QTimer::singleShot(500,
                               [reply,
                               docsPaths,
                               group,
                               &attachments,
                               uploadUrl,
                               &loop,
                               i,
                               id,
                               this]()
            {
                QFile *file = new QFile(docsPaths.at(i));

                if (!file->open(QIODevice::ReadOnly))
                {
                    qCritical() << "File could not be opened:" << docsPaths.at(i);
                    reply->deleteLater();
                    loop.quit();
                    throw "File could not be opened";
                }

                QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

                QHttpPart imagePart;
                imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; "
                                                                                        "name=\"file\"; "
                                                                                        "filename=\"" + file->fileName() + "\""));
                imagePart.setBodyDevice(file);
                file->setParent(multiPart);

                multiPart->append(imagePart);

                QNetworkRequest uploadUrlRequest;
                uploadUrlRequest.setUrl(QUrl(uploadUrl));
                QNetworkReply *transferReply = mNetworkManager->post(uploadUrlRequest, multiPart);

                multiPart->setParent(transferReply);

                QString fileName = QFileInfo(docsPaths.at(i)).baseName();
                connect(transferReply, &QNetworkReply::finished,
                        [transferReply,
                        group,
                        id,
                        &fileName,
                        &attachments,
                        &loop,
                        this]()
                {
                    QTimer::singleShot(500,
                                       [transferReply,
                                       group,
                                       &attachments,
                                       &fileName,
                                       &loop,
                                       id,
                                       this]()
                    {
                        QJsonParseError parseError;
                        const auto data = transferReply->readAll();
                        const auto document = QJsonDocument::fromJson(data, &parseError);

                        if (isJsonErrorReturned(parseError)
                                || isServerErrorReturned(document)
                                || isReplyErrorReturned(*transferReply))
                        {
                            qCritical() << "error uploadUrlRequest finished:"
                                        << parseError.errorString() << " | " << group.name << group.vkid << "|";
                            transferReply->deleteLater();
                            loop.quit();
                            throw "uploadUrlRequest";
                        }

                        const QJsonObject jsonResponse = document.object();

                        qDebug() << "vkApi.docs.save" << jsonResponse;

                        QUrl requestUrl(mVkApiLink
                                        + vkApi.docs.save.method + "?"
                                        );

                        QUrlQuery params;
                        params.addQueryItem("access_token",vkApi.implicitFlowAccessToken);
                        params.addQueryItem("v", vkApi.apiVersion);
                        params.addQueryItem("file", group.vkid);
                        params.addQueryItem("title", fileName);

                        QNetworkRequest request;
                        request.setUrl(requestUrl);
                        request.setHeader(QNetworkRequest::ContentTypeHeader,
                                          "application/x-www-form-urlencoded");

                        QNetworkReply *newReply = mNetworkManager->post(request, params.query().toUtf8());

                        connect(newReply, &QNetworkReply::finished,
                                [newReply,
                                id,
                                group,
                                &attachments,
                                &loop,
                                this]()
                        {
                            QJsonParseError parseError;
                            const auto data = newReply->readAll();
                            const auto document = QJsonDocument::fromJson(data, &parseError);

                            if (isJsonErrorReturned(parseError)
                                    || isServerErrorReturned(document)
                                    || isReplyErrorReturned(*newReply))
                            {
                                qCritical() << "error vkApi.docs.save finished: "
                                            << group.name << group.vkid << "|";
                                newReply->deleteLater();
                                loop.quit();
                                throw "vkApi.docs.save";
                            }

                            const QJsonObject jsonResponse = document.object()["response"].toArray()[0].toObject();
                            qDebug() << "saveWallPhoto" << jsonResponse;

                            attachments += "doc" + QString::number(jsonResponse["owner_id"].toInt()) +
                                    "_" + QString::number(jsonResponse["id"].toInt()) + ",";

                            emit updatedDoc(id);

                            newReply->deleteLater();
                        });

                        transferReply->deleteLater();
                    });
                });

            });

        }
        reply->deleteLater();
    });
   loop.exec();

   return attachments;
}

QString Fetcher::uploadVideosToGroup(const QVector<Path> &videoPaths, QUuid id)
{
    QString attachments = "";

    if (videoPaths.empty()) return attachments;

    int sendedVideos = 0;

    QEventLoop loop;
    QObject *obj = new QObject(this);
    connect(this, &Fetcher::updatedVideo, obj,
            [&sendedVideos,
            videoPaths,
            &loop,
            obj](QUuid id)
    {
        if (++sendedVideos == videoPaths.size())
        {
            obj->deleteLater();
            loop.quit();
        }
    });

    for (std::size_t i = 0; i < videoPaths.size(); i++)
    {
        QTimer::singleShot(500,
                           [videoPaths,
                           &attachments,
                           &loop,
                           i,
                           id,
                           this]()
        {         
            QUrl requestUrl(mVkApiLink
                            + vkApi.videos.save.method + "?"
                            );

            QUrlQuery params;
            params.addQueryItem("access_token",vkApi.implicitFlowAccessToken);
            params.addQueryItem("v", vkApi.apiVersion);
            params.addQueryItem("name", QFileInfo(videoPaths.at(i)).baseName());
            requestUrl.setQuery(params);

            QNetworkRequest request;
            request.setUrl(requestUrl);
            QNetworkReply *transferReply = mNetworkManager->get(request);

            connect(transferReply, &QNetworkReply::finished,
                    [transferReply,
                    videoPaths,
                    id,
                    i,
                    &attachments,
                    &loop,
                    this]()
            {

                QJsonParseError parseError;
                const auto data = transferReply->readAll();
                const auto document = QJsonDocument::fromJson(data, &parseError);

                const QJsonObject jsonServerErrorObject = document.object()["error"].toObject();

                if (isJsonErrorReturned(parseError)
                        || isServerErrorReturned(document)
                        || isReplyErrorReturned(*transferReply))
                {
                    qCritical() << "error uploadUrlRequest finished:"
                                << parseError.errorString() << " | ";
                    transferReply->deleteLater();
                    loop.quit();
                    throw "uploadUrlRequest";
                }

                const QJsonObject jsonResponse = document.object().value("response").toObject();

                QString uploadUrl = jsonResponse["upload_url"].toString();

                qDebug() << "vkApi.videos.save" << jsonResponse;

                QFile *file = new QFile(videoPaths.at(i));

                if (!file->open(QIODevice::ReadOnly))
                {
                    qCritical() << "File could not be opened:" << videoPaths.at(i);
                    transferReply->deleteLater();
                    loop.quit();
                    throw "File could not be opened";
                }

                QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

                QHttpPart imagePart;
                //imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("audio/mpeg"));
                imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; "
                                                                                        "name=\"video_file\"; "
                                                                                        "filename=\"" + file->fileName() + "\""));
                imagePart.setBodyDevice(file);
                file->setParent(multiPart);

                multiPart->append(imagePart);

                QUrlQuery params;
                params.addQueryItem("access_token",vkApi.implicitFlowAccessToken);
                params.addQueryItem("v", vkApi.apiVersion);
                //params.addQueryItem("redirect", jsonResponse["redirect"].toString());

                QNetworkRequest request;
                request.setUrl(QUrl(uploadUrl));

                QNetworkReply *newReply = mNetworkManager->post(request, multiPart);

                multiPart->setParent(newReply);
                QString ownerId = QString::number(jsonResponse["owner_id"].toInt());


                connect(newReply, &QNetworkReply::finished,
                        [newReply,
                        id,
                        &attachments,
                        &loop,
                        this]()
                {
                    QJsonParseError parseError;
                    const auto data = newReply->readAll();
                    const auto document = QJsonDocument::fromJson(data, &parseError);

                    if (isJsonErrorReturned(parseError)
                            || isServerErrorReturned(document)
                            || isReplyErrorReturned(*newReply))
                    {
                        qCritical() << "error vkApi.audios.save finished: " << "|";
                        newReply->deleteLater();
                        loop.quit();
                        throw "vkApi.audios.save";
                    }

                    const QJsonObject jsonResponse = document.object();
                    qDebug() << "save" << jsonResponse;

                    attachments += "video" + QString::number(jsonResponse["owner_id"].toInt()) +
                            "_" + QString::number(jsonResponse["video_id"].toInt()) + ",";

                    emit updatedVideo(id);

                    newReply->deleteLater();
                });

                transferReply->deleteLater();
            });
        });
    }

   loop.exec();

   return attachments;
}
