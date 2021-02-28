#include "fetcher.h"
#include <types.h>

#include <iostream>

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
#include <QDateTime>
#include <QEventLoop>
#include <QTimer>
#include <QHttpPart>
#include <QHttpMultiPart>
#include <QMessageBox>
#include <QMessageLogger>

static Request vkApi;

Fetcher::Fetcher(QObject *parent) : QObject(parent)
{
    mNetworkManager = new QNetworkAccessManager(this);
}

void Fetcher::setRepository(const std::shared_ptr<Repository> repository)
{
    mRepository = repository;
}

void Fetcher::onGroupDataNeed(const std::vector<Link> links)
{
    vkApi.groupById.groupIds = "";
    for (const auto &link : links)
        vkApi.groupById.groupIds = vkApi.groupById.groupIds + link + ",";

    QUrl requestUrl(mVkApiLink
                    + vkApi.groupById.method);

    QUrlQuery params;
    params.addQueryItem("access_token", vkApi.authorizationAccessToken);
    params.addQueryItem("v", vkApi.apiVersion);
    params.addQueryItem("fields", vkApi.groupById.fields);
    params.addQueryItem("group_ids", vkApi.groupById.groupIds);
    requestUrl.setQuery(params.query());

    QNetworkRequest request;
    request.setUrl(requestUrl);

    QNetworkReply *reply = mNetworkManager->get(request);

    connect(reply, &QNetworkReply::errorOccurred,
            [reply, this]()
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            qCritical() << "vkApi.groupById error:" << reply->errorString();
            emit errorGroupFetch("ERROR: " + reply->errorString());
        }
    });

    connect(reply, &QNetworkReply::finished,
            [reply, this]()
    {
        QJsonParseError parseError;
        const auto data = reply->readAll();
        const auto document = QJsonDocument::fromJson(data, &parseError);

        const QJsonObject jsonServerErrorObject = document.object()["error"].toObject();

        if (QJsonParseError::NoError != parseError.error)
        {
            qCritical() << "reply vkApi.groupById finished:" << parseError.errorString();
            emit errorGroupFetch("ERROR: " + reply->errorString());
            reply->deleteLater();
            return;
        }

        if (!jsonServerErrorObject.isEmpty())
        {
            qCritical() << "vkApi.groupById - server error:" << jsonServerErrorObject;
            emit errorGroupFetch("Ошибка на стороне сервера.");
            reply->deleteLater();
            return;
        }

        const QJsonArray jsonResponse = document.object()["response"].toArray();
        qDebug() << jsonResponse;

        std::vector<Group> groups;
        Id id = 0;
        foreach (const QJsonValue &value, jsonResponse)
        {
            QJsonObject groupInfo = value.toObject();

            Group group;
            group.id = ++id;
            group.vkid = QString::number(groupInfo["id"].toDouble(), 'f', 0);
            group.name = groupInfo["name"].toString();
            group.link = groupInfo["screen_name"].toString();
            group.photo = groupInfo["photo_50"].toString();
            group.membersCount = groupInfo["members_count"].toDouble();
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
            params.addQueryItem("access_token", vkApi.authorizationAccessToken);
            params.addQueryItem("v", vkApi.apiVersion);
            params.addQueryItem("group_id", group.vkid);
            getWallUpdateRequestUrl.setQuery(params);

            QNetworkRequest getWallUpdateRequest;
            getWallUpdateRequest.setUrl(getWallUpdateRequestUrl);

            QNetworkReply *reply = mNetworkManager->post(getWallUpdateRequest, params.query().toUtf8());

            connect(reply, &QNetworkReply::errorOccurred,
                    [reply, this]()
            {
                if (reply->error() != QNetworkReply::NoError)
                {
                    qCritical() << "vkApi.photos.getWallUploadServer error:" << reply->errorString();
                    emit errorMessageSend("ERROR: " + reply->errorString());
                    reply->deleteLater();
                    throw "vkApi.photos.getWallUploadServer";
                }

            });

            connect(reply, &QNetworkReply::finished,
                    [reply, group, loop, this]()
            {
                QString uploadUrl;
                QJsonParseError parseError;
                const auto data = reply->readAll();
                const auto document = QJsonDocument::fromJson(data, &parseError);

                const QJsonObject jsonServerErrorObject = document.object()["error"].toObject();

                if (QJsonParseError::NoError != parseError.error)
                {
                    qCritical() << "error reply vkApi.photos.getWallUploadServer finished:"
                                << parseError.errorString() << " | " << group.name << group.vkid << "|";
                    emit errorMessageSend("Не удалось загрузить изображение для " + group.name);
                    reply->deleteLater();
                    throw "vkApi.photos.getWallUploadServer";
                }

                if (!jsonServerErrorObject.isEmpty())
                {
                    qCritical() << "vkApi.photos.getWallUploadServer - server error:"
                                << jsonServerErrorObject << " | " << group.name << group.vkid << "|";;
                    emit errorMessageSend("Не удалось загрузить изображение для " + group.name);
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
                            emit errorMessageSend("Не удалось загрузить изображение для " + group.name);
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

                        connect(transferReply, &QNetworkReply::errorOccurred,
                                [transferReply, this]()
                        {
                            if (transferReply->error() != QNetworkReply::NoError)
                            {
                                qCritical() << "vkApi.photos.getWallUploadServer error:" << transferReply->errorString();
                                emit errorMessageSend("ERROR: " + transferReply->errorString());
                                transferReply->deleteLater();
                                throw "vkApi.photos.getWallUploadServer";
                            }

                        });

                        multiPart->setParent(transferReply);
                        connect(transferReply, &QNetworkReply::finished,
                                [transferReply, group, this]()
                        {
                            QTimer::singleShot(500,
                                               [transferReply, group, this]()
                            {
                                //!обработка ошибок reply
                                QJsonParseError parseError;
                                const auto data = transferReply->readAll();
                                const auto document = QJsonDocument::fromJson(data, &parseError);

                                const QJsonObject jsonServerErrorObject = document.object()["error"].toObject();

                                if (QJsonParseError::NoError != parseError.error)
                                {
                                    qCritical() << "error reply uploadUrlRequest finished:"
                                                << parseError.errorString() << " | " << group.name << group.vkid << "|";
                                    emit errorMessageSend("Не удалось загрузить изображение для " + group.name);
                                    transferReply->deleteLater();
                                    throw "uploadUrlRequest";
                                }

                                if (!jsonServerErrorObject.isEmpty())
                                {
                                    qCritical() << "uploadUrlRequest - server error:"
                                                << jsonServerErrorObject << " | " << group.name << group.vkid << "|";;
                                    emit errorMessageSend("Не удалось загрузить изображение для " + group.name);
                                    transferReply->deleteLater();
                                    throw "uploadUrlRequest";
                                }

                                const QJsonObject jsonResponse = document.object();

                                qDebug() << "vkApi.photos.saveWallPhoto" << jsonResponse;

                                QUrl requestUrl(mVkApiLink
                                                + vkApi.photos.saveWallPhoto.method + "?"
                                                );

                                QUrlQuery params;
                                params.addQueryItem("access_token",vkApi.authorizationAccessToken);
                                params.addQueryItem("v", vkApi.apiVersion);
                                params.addQueryItem("group_id", group.vkid);
                                params.addQueryItem("hash", jsonResponse["hash"].toString());
                                params.addQueryItem("server", QString::number(jsonResponse["server"].toInt()));
                                params.addQueryItem("photo", jsonResponse["photo"].toString().toUtf8());

                                QNetworkRequest request;
                                request.setUrl(requestUrl);

                                QNetworkReply *newReply = mNetworkManager->post(request, params.query().toUtf8());

                                connect(newReply, &QNetworkReply::errorOccurred,
                                        [newReply, this]()
                                {
                                    if (newReply->error() != QNetworkReply::NoError)
                                    {
                                        qCritical() << "uploadUrlRequest error:" << newReply->errorString();
                                        emit errorMessageSend("ERROR: " + newReply->errorString());
                                        newReply->deleteLater();
                                        throw "uploadUrlRequest";
                                    }

                                });

                                connect(newReply, &QNetworkReply::finished,
                                        [newReply, group, this]()
                                {
                                    QJsonParseError parseError;
                                    const auto data = newReply->readAll();
                                    const auto document = QJsonDocument::fromJson(data, &parseError);

                                    const QJsonObject jsonServerErrorObject = document.object()["error"].toObject();

                                    if (QJsonParseError::NoError != parseError.error)
                                    {
                                        qCritical() << "error reply vkApi.photos.saveWallPhoto finished:"
                                                    << parseError.errorString() << " | " << group.name << group.vkid << "|";
                                        emit errorMessageSend("Не удалось загрузить изображение для " + group.name);
                                        newReply->deleteLater();
                                        throw "vkApi.photos.saveWallPhoto";
                                    }

                                    if (!jsonServerErrorObject.isEmpty())
                                    {
                                        qCritical() << "vkApi.photos.saveWallPhoto - server error:"
                                                    << jsonServerErrorObject << " | " << group.name << group.vkid << "|";;
                                        emit errorMessageSend("Не удалось загрузить изображение для " + group.name);
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

                    connect(wallPostReply, &QNetworkReply::errorOccurred,
                            [wallPostReply, this]()
                    {
                        if (wallPostReply->error() != QNetworkReply::NoError)
                        {
                            qCritical() << "vkApi.wallPost error:" << wallPostReply->errorString();
                            emit errorMessageSend("ERROR: " + wallPostReply->errorString());
                            wallPostReply->deleteLater();
                            throw "vkApi.wallPost";
                        }

                    });

                    connect(wallPostReply, &QNetworkReply::finished,
                            [wallPostReply, group, this]()
                    {
                        QJsonParseError parseError;
                        const auto data = wallPostReply->readAll();
                        const auto document = QJsonDocument::fromJson(data, &parseError);

                        const QJsonObject jsonServerErrorObject = document.object()["error"].toObject();

                        if (QJsonParseError::NoError != parseError.error)
                        {
                            qCritical() << "reply vkApi.photos.saveWallPhoto finished:"
                                        << parseError.errorString() << " | " << group.name << group.vkid << "|";
                            emit errorMessageSend("Не удалось отправить пост для " + group.name);
                            wallPostReply->deleteLater();
                            throw "vkApi.photos.saveWallPhoto";
                        }

                        if (!jsonServerErrorObject.isEmpty())
                        {
                            qCritical() << "vkApi.photos.saveWallPhoto - server error:"
                                        << jsonServerErrorObject << " | " << group.name << group.vkid << "|";;
                            emit errorMessageSend("Не удалось отправить пост для " + group.name);
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

Fetcher::~Fetcher()
{
}
