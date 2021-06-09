#ifndef WAITINGLISTWIDGETITEMEDIT_H
#define WAITINGLISTWIDGETITEMEDIT_H

#include <QWidget>
#include <QMenu>
#include <QDebug>
#include <QListWidgetItem>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>

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
    void backButtonReleased();
    void saveButtonReleased();
    void cancelButtonReleased();

public:
    explicit WaitingListWidgetItemEdit(QWidget *parent = nullptr);
    ~WaitingListWidgetItemEdit();

    void setFetcher(const std::shared_ptr<Fetcher> fetcher);

private slots:
    void onGroupListWidgetItemClicked(QListWidgetItem *item);
    void onBackButtonReleased();
    void onSaveButtonReleased();
    void onCancelButtonReleased();

private:
    void addUserGroupListItem(const Group &group);
    QPixmap roundPhoto35(QPixmap photo) const;

    void setUserGroups(const QVector<Group> &groups);

    void createGroupListView();

private:
    Ui::WaitingListWidgetItemEdit *ui;    

    GroupListView *mGroupList = nullptr;
    std::shared_ptr<Fetcher> mFetcher = nullptr;
};

#endif // WAITINGLISTWIDGETITEMEDIT_H
