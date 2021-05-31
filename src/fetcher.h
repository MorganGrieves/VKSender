#ifndef FETCHER_H
#define FETCHER_H

#include <QObject>
#include <QString>
#include <QNetworkReply>
#include <QPixmap>
#include <QJsonParseError>

#include "types.h"
#include "repository.h"

struct Request
{
    struct GroupById
    {
        QString method = "groups.getById";
        QString groupIds = "";
        QString fields = "description,members_count";
    };

    struct WallPost
    {
        QString method = "wall.post";
        QString groupIds = "";
        QString description = "";
        QString message = "";
        QString groupId = "";
        QString attachments = "";
    };
    struct Photos
    {
        struct getWallUploadServer
        {
            QString method = "photos.getWallUploadServer";
            QString groupId = "";
        };

        struct saveWallPhoto
        {
            QString method = "photos.saveWallPhoto";
            QString hash = "";
            QString photo = "";
            int server = 0;
            QString groupId = "";
        };

        getWallUploadServer getWallUploadServer;
        saveWallPhoto saveWallPhoto;

    };

    struct UsersGet
    {
        QString method = "users.get";
        QString user_ids = "";
        QString fields = "";
        QString name_case = "";
    };

    struct WallDelete
    {
        QString method = "wall.delete";
        QString ownerId = "";
        QString postId = "";
    };

    GroupById groupById;
    WallPost wallPost;
    Photos photos;
    WallDelete wallDelete;
    UsersGet usersGet;

    QString implicitFlowAccessToken = "";
    QString apiVersion = "5.130";
};

class Fetcher : public QObject
{
    Q_OBJECT

signals:
    void updatedGroupData(QVector<Group> groups);
    void updatedPhoto();
    void sentMessage(Group group);
    void deletedPost(QString postId, QString ownerId);
    void userPhoto100Update();
    void userNameUpdate();

public:
    explicit Fetcher(QObject *parent = nullptr);
    ~Fetcher();
    void setRepository(const std::shared_ptr<Repository> repository);   
    bool tokenIsEmpty() const;
    void setAccessToken(QString token);

    const QPixmap &getUserPhoto100() const;
    const QString &getUserName() const;

public slots:
    void onGroupDataNeed(const std::vector<Link> links);
    void onMessageSent(const QString messageText, const std::vector<Path> photoPaths);
    void onPostDelete(const QString postId, const QString ownerId);
    void onUserDataUpdate();

private:
    bool isReplyErrorReturned(const QNetworkReply &reply);
    bool isJsonErrorReturned(const QJsonParseError &error);
    bool isServerErrorReturned(const QJsonDocument &document);

    void setUserPhoto100(const QPixmap &userphoto);
    void setUserName(const QString &userName);

    void downloadUserInfo();
    QVector<Group> downloadUserGroups();

private:
    const Link mVkApiLink = "https://api.vk.com/method/";
    const Link mVkLink = "https://vk.com";
    QNetworkAccessManager *mNetworkManager;

    QString mUserName;
    QPixmap mUserPhoto100;

    QVector<Group> mUserGroups;

    std::shared_ptr<Repository> mRepository = nullptr;

    std::vector<Path> mPhotoPaths;
};

#endif // FETCHER_H
