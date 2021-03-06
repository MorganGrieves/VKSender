#ifndef FETCHER_H
#define FETCHER_H

#include <QObject>
#include <QString>
#include <QNetworkReply>

#include "types.h"
#include "repository.h"

struct Request
{
    struct GroupById
    {
        QString method = "groups.getById";
        QString groupIds = "";
        QString fields = "desciption,members_count";
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

    GroupById groupById;
    WallPost wallPost;
    Photos photos;

    QString userId = "112836979";
    QString implicitFlowAccessToken = "";
    QString apiVersion = "5.130";
};

class Fetcher : public QObject
{
    Q_OBJECT

public:
    explicit Fetcher(QObject *parent = nullptr);
    ~Fetcher();
    void setRepository(const std::shared_ptr<Repository> repository);   
    bool tokenIsEmpty() const;
    void setAccessToken(QString token);

signals:
    void updatedGroupData(QVector<Group> groups);
    void updatedPhoto();
    void sentMessage(Group group);

    void errorGroupFetch(QString err);
    void errorMessageSend(QString err);

public slots:
    void onGroupDataNeed(const std::vector<Link> links);
    void onMessageSent(const QString messageText, const std::vector<Path> photoPaths);

private:
    const Link mVkApiLink = "https://api.vk.com/method/";
    const Link mVkLink = "https://vk.com";
    QNetworkAccessManager *mNetworkManager;

    std::shared_ptr<Repository> mRepository = nullptr;

    std::vector<Path> mPhotoPaths;
};

#endif // FETCHER_H
