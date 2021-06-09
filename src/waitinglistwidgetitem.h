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
    void launchButtonReleased();
    void waitingListWidgetItemReleased(WaitingListWidgetItemEdit *item);

public:
    explicit WaitingListWidgetItem(QWidget *parent = nullptr);
    ~WaitingListWidgetItem();

    void showItemEdit();

    void setFetcher(const std::shared_ptr<Fetcher> fetcher);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onDeleteButtonReleased();
    void onLaunchButtonReleased();

private:
    Ui::WaitingListWidgetItem *ui;

    WaitingListWidgetItemEdit *mEditItem = nullptr;

    std::shared_ptr<Fetcher> mFetcher = nullptr;
};

#endif // WAITINGLISTWIDGETITEM_H
