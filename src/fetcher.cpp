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

void Fetcher::sendMessage(const MessagePack &pack)
{
    QEventLoop *groupLoop = new QEventLoop(this);
    connect(this, &Fetcher::sentMessage, groupLoop, &QEventLoop::quit);

    QEventLoop *loop = new QEventLoop(this);
    connect(this, &Fetcher::updatedPhoto, loop, &QEventLoop::quit);

    for (const auto& group : pack.groups)
    {
        if (group.second == Qt::Unchecked)
            continue;

        try
        {
            QString attachments = "";

            QUrl getWallUpdateRequestUrl(mVkApiLink
                            + vkApi.photos.getWallUploadServer.method
                            );
            QUrlQuery params;
            params.addQueryItem("access_token", vkApi.implicitFlowAccessToken);
            params.addQueryItem("v", vkApi.apiVersion);
            params.addQueryItem("group_id", group.first.vkid);
            getWallUpdateRequestUrl.setQuery(params);

            QNetworkRequest getWallUpdateRequest;
            getWallUpdateRequest.setUrl(getWallUpdateRequestUrl);
            getWallUpdateRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                                           "application/x-www-form-urlencoded");

            QNetworkReply *reply = mNetworkManager->post(getWallUpdateRequest, params.query().toUtf8());

            connect(reply, &QNetworkReply::finished,
                    [reply,
                    pack,
                    group,
                    &attachments,
                    loop,
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
                                << parseError.errorString() << " | " << group.first.name << group.first.vkid << "|";
                    reply->deleteLater();
                    throw "vkApi.photos.getWallUploadServer";
                }

                QJsonObject serverInfo = document.object().value("response").toObject();

                qDebug() << "vkApi.photos.getWallUploadServer" << serverInfo;

                uploadUrl = serverInfo["upload_url"].toString();

                for (std::size_t i = 0; i < pack.photoPaths.size(); i++)
                {
                    QTimer::singleShot(500,
                                       [reply,
                                       pack,
                                       group,
                                       &attachments,
                                       uploadUrl,
                                       i,
                                       this]()
                    {
                        QFile *file = new QFile(pack.photoPaths.at(i));

                        if (!file->open(QIODevice::ReadOnly))
                        {
                            qCritical() << "File could not be opened:" << pack.photoPaths.at(i);
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
                                [transferReply,
                                group,
                                &attachments,
                                pack,
                                this]()
                        {
                            QTimer::singleShot(500,
                                               [transferReply,
                                               group,
                                               &attachments,
                                               pack,
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
                                                << parseError.errorString() << " | " << group.first.name << group.first.vkid << "|";
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
                                params.addQueryItem("group_id", group.first.vkid);
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
                                        pack,
                                        group,
                                        &attachments,
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
                                                    << group.first.name << group.first.vkid << "|";
                                        newReply->deleteLater();
                                        throw "vkApi.photos.saveWallPhoto";
                                    }

                                    const QJsonObject jsonResponse = document.object()["response"].toArray()[0].toObject();
                                    qDebug() << "saveWallPhoto" << jsonResponse;
                                    QUrl requestUrl(mVkApiLink
                                                    + vkApi.wallPost.method + "?"
                                                     );

                                    attachments += "photo" + QString::number(jsonResponse["owner_id"].toInt()) +
                                            "_" + QString::number(jsonResponse["id"].toInt()) + ",";

                                    emit updatedPhoto(pack.id);

                                    newReply->deleteLater();
                                });

                                transferReply->deleteLater();
                            });
                        });

                    });

                    loop->exec();
                }

                QTimer::singleShot(500,
                                   [group,
                                   pack,
                                   &attachments,
                                   this]()
                {
                    QUrl requestUrl(mVkApiLink
                                    + vkApi.wallPost.method + "?"
                                     );

                    QUrlQuery params;
                    params.addQueryItem("access_token", vkApi.implicitFlowAccessToken);
                    params.addQueryItem("v", vkApi.apiVersion);
                    params.addQueryItem("message", pack.message);
                    params.addQueryItem("owner_id", "-" + group.first.vkid);
                    params.addQueryItem("attachments", attachments);
                    requestUrl.setQuery(params.query());

                    QNetworkRequest request;
                    request.setUrl(requestUrl);

                    QNetworkReply *wallPostReply = mNetworkManager->get(request);

                    connect(wallPostReply, &QNetworkReply::finished,
                            [wallPostReply,
                            group,
                            pack,
                            &attachments,
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
                                        << group.first.name << group.first.vkid << "|";
                            wallPostReply->deleteLater();
                            throw "vkApi.photos.saveWallPhoto";
                        }

                        QString postIdInfo = QString::number(document.object().value("response").toObject()["post_id"].toInt());

                        qDebug() << "vkApi.photos.getWallUploadServer" << postIdInfo;

                        emit sentMessage(pack.id, group.first, postIdInfo);
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
            group.canPost = groupInfo["can_post"].toInt() ? true : false;

            if (!group.canPost)
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
