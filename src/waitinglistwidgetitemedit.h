#ifndef WAITINGLISTWIDGETITEMEDIT_H
#define WAITINGLISTWIDGETITEMEDIT_H

#include <QWidget>
#include <QMenu>
#include <QDebug>
#include <QListWidgetItem>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QFileDialog>
#include <QFileInfo>

#include "types.h"
#include "grouplistview.h"
#include "fetcher.h"

namespace Ui {
class WaitingListWidgetItemEdit;
}

class WaitingListWidgetItemEdit : public QWidget
{
    Q_OBJECT

signals:
    void launchButtonReleased();
    void saveButtonReleased();
    void cancelButtonReleased();
    void backButtonReleased();

public:
    enum DataRoles
    {
        PathRole = Qt::UserRole,
        DataRole
    };

    explicit WaitingListWidgetItemEdit(QWidget *parent = nullptr);
    WaitingListWidgetItemEdit(const WaitingListWidgetItemEdit & item);
    ~WaitingListWidgetItemEdit();

    void setFetcher(const std::shared_ptr<Fetcher> fetcher);

    QString getPackName() const;
    void setMessagePack(const MessagePack *message);

    int getCheckedGroupsNumber();
    MessagePack getMessageInfo() const;

    void setId(const QUuid &id);

private slots:
    void onGroupListWidgetItemClicked(QListWidgetItem *item);
    void onSaveButtonReleased();
    void onCancelButtonReleased();
    void onUserGroupsUpdate();
    void onPhotoListWidgetDoubleClicked(const QModelIndex &index);
    void onVideoListWidgetDoubleClicked(const QModelIndex &index);
    void onAudioListWidgetDoubleClicked(const QModelIndex &index);
    void onDocListWidgetDoubleClicked(const QModelIndex &index);
    void onDeleteSelectedButtonReleased();
    void onAddGroupButtonReleased();
    void onBackButtonReleased();
    void onLaunchButtonReleased();

private:
    void addUserGroupListItem(const Group &group, Qt::CheckState state = Qt::Unchecked);
    QPixmap roundPhoto35(QPixmap photo) const;
    void setUserGroups(const QVector<Group> &groups);

    void createGroupListView();

    QString filterGroupLineEdit(const QString &text);

    int countAllAttachments() const;
    bool canAddAttachment() const;

    bool photoIncludes(const QString &fileName);
    bool audioIncludes(const QString &fileName);
    bool videoIncludes(const QString &fileName);
    bool docIncluded(const QString &fileName);

    bool acceptDocFile(const QString &fileName) const;
    bool isAttchmentsEmpty() const;

private:
    Ui::WaitingListWidgetItemEdit *ui;    

    GroupListView *mGroupList = nullptr;
    std::shared_ptr<Fetcher> mFetcher = nullptr;

    QAction *mPhotoAction = nullptr;
    QAction *mVideoAction = nullptr;
    QAction *mDocAction = nullptr;
    QAction *mAudioAction = nullptr;

    bool mSaveFlag = false;

    QUuid mId;
};

#endif // WAITINGLISTWIDGETITEMEDIT_H
