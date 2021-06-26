#ifndef WAITINGWIDGET_H
#define WAITINGWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QMessageBox>

#include "waitinglistwidgetitem.h"
#include "waitinglistwidgetitemedit.h"
#include "fetcher.h"
#include "repository.h"

namespace Ui {
class WaitingWidget;
}

class WaitingWidget : public QWidget
{
    Q_OBJECT

signals:
    void waitingListWidgetItemReleased(WaitingListWidgetItemEdit *item);
    void showWidget();
    void launchSending(MessagePack message);
    void loadListsButtonRelease();
    void saveListsButtonRelease();

public:
    explicit WaitingWidget(QWidget *parent = nullptr);
    ~WaitingWidget();

    void setFetcher(const std::shared_ptr<Fetcher> fetcher);
    void setRepository(const std::shared_ptr<Repository> repository);

    void addListItem(MessagePack message);

    int listSize() const;

    void loadLists();
    void saveLists();

private:
    QVector<MessagePack> getAllMessagePacks() const;
    void setAllMessagePacks(const QVector<MessagePack> &messages);

private slots:  
    void onLoadListsButtonReleased();
    void onSaveListsButtonReleased();
    void onDeleteButtonReleased();
    void onAddListButtonReleased();
    void onLaunchSending(MessagePack message);

private:
    Ui::WaitingWidget *ui;

    QString mLoadedListPath = "";

    std::shared_ptr<Fetcher> mFetcher = nullptr;
    std::shared_ptr<Repository> mRepository = nullptr;
};

#endif // WAITINGWIDGET_H
