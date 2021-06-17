#ifndef FINISHEDLISTWIDGETITEM_H
#define FINISHEDLISTWIDGETITEM_H

#include <QWidget>
#include <QMouseEvent>

#include "fetcher.h"
#include "finishedlistwidgetitemedit.h"

namespace Ui {
class FinishedListWidgetItem;
}

class FinishedListWidgetItem : public QWidget
{
    Q_OBJECT

signals:
    void finishedListWidgetItemReleased(FinishedListWidgetItemEdit *item);
    void backToWaiting(MessagePack message);

public:
    explicit FinishedListWidgetItem(QWidget *parent = nullptr);
    ~FinishedListWidgetItem();

    void setFetcher(const std::shared_ptr<Fetcher> fetcher);
    void setSendingResult(SendingResult result);
    void showItemEdit();

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onBackToWaitingButtonReleased();

private:
    Ui::FinishedListWidgetItem *ui;

    FinishedListWidgetItemEdit *mFinishedEdit = nullptr;
    std::shared_ptr<Fetcher> mFetcher = nullptr;
    SendingResult mSendingResult;
};

#endif // FINISHEDLISTWIDGETITEM_H
