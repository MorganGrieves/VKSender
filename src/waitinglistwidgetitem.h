#ifndef WAITINGLISTWIDGETITEM_H
#define WAITINGLISTWIDGETITEM_H

#include <QWidget>
#include <QMouseEvent>

#include "waitinglistwidgetitemedit.h"
#include "fetcher.h"

namespace Ui {
class WaitingListWidgetItem;
}

class WaitingListWidgetItem : public QWidget
{
    Q_OBJECT

signals:
    void deleteButtonReleased();
    void waitingListWidgetItemReleased(WaitingListWidgetItemEdit *item);
    void showWaitingWidget();
    void launchSending(MessagePack pack);

public:
    explicit WaitingListWidgetItem(QWidget *parent = nullptr);
    ~WaitingListWidgetItem();

    void showItemEdit();

    void setFetcher(const std::shared_ptr<Fetcher> fetcher);
    void setMessagePack(const MessagePack *message);
    MessagePack getMessagePack() const;

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onDeleteButtonReleased();
    void onSaveButtonReleased();
    void onCancelButtonReleased();
    void onLaunchButtonReleased();
    void onBackButtonReleased();

private:
    Ui::WaitingListWidgetItem *ui;

    WaitingListWidgetItemEdit *mEditItem = nullptr;
    WaitingListWidgetItemEdit *mTmpEditItem = nullptr;

    std::shared_ptr<Fetcher> mFetcher = nullptr;

    QUuid mId;
};

#endif // WAITINGLISTWIDGETITEM_H
