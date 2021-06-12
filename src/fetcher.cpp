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
//    vkApi.implicitFlowAccessToken = settings.value("Fetcher/vkApi.implicitFlowAccessToken").toString();
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
    return (vkApi.implicitFlowAccessToken == "");
}

void Fetcher::setAccessToken(QString token)
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
        group.photo50 = *uploadPhoto(QUrl(group.photo50Link));
        group.canPost = jsonResponse["can_post"].toBool();

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

void Fetcher::onGroupDataNeed(const std::vector<Link> links)
{
    vkApi.groupById.groupIds = "";
    for (const auto &link : links)
        vkApi.groupById.groupIds = vkApi.groupById.groupIds + link + ",";

    QUrl requestUrl(mVkApiLink
                    + vkApi.groupById.method);

    QUrlQuery params;
    params.addQueryItem("access_token", vkApi.implicitFlowAccessToken);
    params.addQueryItem("v", vkApi.apiVersion);
    params.addQueryItem("fields", vkApi.groupById.fields);
    params.addQueryItem("group_ids", vkApi.groupById.groupIds);
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
            qDebug() << "error vkApi.groupById.groupIds";
            reply->deleteLater();
            return;
        }

        const QJsonArray jsonResponse = document.object()["response"].toArray();
        qDebug() << jsonResponse;

        QVector<Group> groups;

        foreach (const QJsonValue &value, jsonResponse)
        {
            QJsonObject groupInfo = value.toObject();

            Group group;
            group.vkid = QString::number(groupInfo["id"].toDouble(), 'f', 0);
            group.name = groupInfo["name"].toString();
            group.screenName = groupInfo["screen_name"].toString();
            group.photo50Link = groupInfo["photo_50"].toString();
            group.photo50 = *uploadPhoto(QUrl(group.photo50Link));
            group.canPost = groupInfo["can_post"].toBool();
            groups.push_back(group);
        }

        emit updatedGroupData(groups);       
        reply->deleteLater();
    });

}

void Fetcher::onMessageSent(const QString messageText, const std::vector<Path> photoPaths)
{
    vkApi.wallPost.message = messageText;
    mPhotoPaths = photoPaths;

    QEventLoop *groupLoop = new QEventLoop(this);
    connect(this, &Fetcher::sentMessage, groupLoop, &QEventLoop::quit);

    QEventLoop *loop = new QEventLoop(this);
    connect(this, &Fetcher::updatedPhoto, loop, &QEventLoop::quit);

    for (const auto& group : mRepository->getGroupData())
    {
        try
        {
            vkApi.wallPost.attachments = "";

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

            QNetworkReply *reply = mNetworkManager->post(getWallUpdateRequest, params.query().toUtf8());

            connect(reply, &QNetworkReply::finished,
                    [reply, group, loop, this]()
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
                    throw "vkApi.photos.getWallUploadServer";
                }

                QJsonObject serverInfo = document.object().value("response").toObject();

                qDebug() << "vkApi.photos.getWallUploadServer" << serverInfo;

                uploadUrl = serverInfo["upload_url"].toString();

                for (std::size_t i = 0; i < mPhotoPaths.size(); i++)
                {

                    QTimer::singleShot(500,
                                       [this, i, group, uploadUrl, reply]()
                    {
                        QFile *file = new QFile(mPhotoPaths.at(i));

                        if (!file->open(QIODevice::ReadOnly))
                        {
                            qCritical() << "File could not be opened:" << mPhotoPaths.at(i);
                            reply->deleteLater();
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
                                [transferReply, group, this]()
                        {
                            QTimer::singleShot(500,
                                               [transferReply, group, this]()
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

                                QNetworkReply *newReply = mNetworkManager->post(request, params.query().toUtf8());

                                connect(newReply, &QNetworkReply::finished,
                                        [newReply, group, this]()
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
                                        throw "vkApi.photos.saveWallPhoto";
                                    }

                                    const QJsonObject jsonResponse = document.object()["response"].toArray()[0].toObject();
                                    qDebug() << "saveWallPhoto" << jsonResponse;
                                    QUrl requestUrl(mVkApiLink
                                                    + vkApi.wallPost.method + "?"
                                                     );

                                    vkApi.wallPost.attachments += "photo" + QString::number(jsonResponse["owner_id"].toInt()) +
                                            "_" + QString::number(jsonResponse["id"].toInt()) + ",";

                                    emit updatedPhoto();

                                    newReply->deleteLater();
                                });

                                transferReply->deleteLater();
                            });
                        });

                    });

                    loop->exec();
                }

                QTimer::singleShot(500,
                                   [this, group]()
                {
                    QUrl requestUrl(mVkApiLink
                                    + vkApi.wallPost.method + "?"
                                     );

                    QUrlQuery params;
                    params.addQueryItem("access_token", vkApi.implicitFlowAccessToken);
                    params.addQueryItem("v", vkApi.apiVersion);
                    params.addQueryItem("message", vkApi.wallPost.message);
                    params.addQueryItem("owner_id", "-" + group.vkid);
                    params.addQueryItem("attachments", vkApi.wallPost.attachments);
                    requestUrl.setQuery(params.query());

                    QNetworkRequest request;
                    request.setUrl(requestUrl);

                    QNetworkReply *wallPostReply = mNetworkManager->get(request);

                    connect(wallPostReply, &QNetworkReply::finished,
                            [wallPostReply, group, this]()
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
                            throw "vkApi.photos.saveWallPhoto";
                        }

                        qDebug() << "wall post: " << wallPostReply->readAll();
                        emit sentMessage(group);
                        wallPostReply->deleteLater();
                    });

                });

                reply->deleteLater();
            });

            groupLoop->exec();
        }
        catch (...)
        {
            groupLoop->quit();
            loop->quit();
            continue;
        }
    }
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
                        << postId << ownerId;
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
        qCritical() << "jsonErrorReturned";
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
            qCritical() << "error vkApi.usersGet  finished:";
            reply->deleteLater();
            return;
        }

        const QJsonArray jsonResponse = document.object()["response"].toArray();
        qDebug() << "vk.usersGet" << jsonResponse;

        setUserName(jsonResponse.at(0).toObject()["last_name"].toString() + " "
                + jsonResponse.at(0).toObject()["first_name"].toString());
        setUserId(jsonResponse.at(0).toObject()["id"].toInt());
        setUserPhoto100(*uploadPhoto(QUrl(jsonResponse.at(0).toObject()["photo_100"].toString())));
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
        qDebug() << jsonResponse["count"].toInt();
        const QJsonArray groupJsonArray = jsonResponse["items"].toArray();

        foreach (const QJsonValue &value, groupJsonArray)
        {
            QJsonObject groupInfo = value.toObject();

            Group group;
            group.vkid = QString::number(groupInfo["id"].toDouble(), 'f', 0);
            group.name = groupInfo["name"].toString();
            group.screenName = groupInfo["screen_name"].toString();
            group.photo50Link = groupInfo["photo_50"].toString();
            group.photo50 = *uploadPhoto(QUrl(group.photo50Link));
            group.canPost = groupInfo["can_post"].toBool();

            if (group.canPost)
                continue;

            groups.push_back(group);
        }

        setUserGroups(groups);
        reply->deleteLater();
    });
}

QPixmap *Fetcher::uploadPhoto(const QUrl &requestUrl) const
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
