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
    explicit WaitingListWidgetItemEdit(QWidget *parent = nullptr);
    WaitingListWidgetItemEdit(const WaitingListWidgetItemEdit & item);
    ~WaitingListWidgetItemEdit();

    void setFetcher(const std::shared_ptr<Fetcher> fetcher);
    QString getPackName() const;
    void setMessagePack(MessagePack message);
    int getCheckedGroupsNumber();
    MessagePack getMessageInfo() const;

private slots:
    void onGroupListWidgetItemClicked(QListWidgetItem *item);
    void onSaveButtonReleased();
    void onCancelButtonReleased();
    void onUserGroupsUpdate();
    void onPhotosListViewDoubleClicked(const QModelIndex &index);
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

private:
    Ui::WaitingListWidgetItemEdit *ui;    

    GroupListView *mGroupList = nullptr;
    std::shared_ptr<Fetcher> mFetcher = nullptr;

    QAction *mPhotoAction = nullptr;
    QAction *mVideoAction = nullptr;
    QAction *mFileAction = nullptr;

    bool mSaveFlag = false;

    std::map<Path, QListWidgetItem *> mPhotoPaths;
};

#endif // WAITINGLISTWIDGETITEMEDIT_H
