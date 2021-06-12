#ifndef WAITINGWIDGET_H
#define WAITINGWIDGET_H

#include <vector>

#include <QWidget>
#include <QListWidgetItem>

#include "waitinglistwidgetitem.h"
#include "waitinglistwidgetitemedit.h"
#include "fetcher.h"

namespace Ui {
class WaitingWidget;
}

class WaitingWidget : public QWidget
{
    Q_OBJECT

signals:
    void waitingListWidgetItemReleased(WaitingListWidgetItemEdit *item);
    void showWidget();

public:
    explicit WaitingWidget(QWidget *parent = nullptr);
    ~WaitingWidget();

    void setFetcher(const std::shared_ptr<Fetcher> fetcher);

private slots:
    void onDeleteButtonReleased();
    void onAddListButtonReleased();
    void onLoadListsButtonReleased();
    void onLaunchButtonReleased();
    void onWaitingListWidgetItemEditSaved();

private:
    Ui::WaitingWidget *ui;

    std::vector<WaitingListWidgetItem *> mWaitingListItemVector;
    std::shared_ptr<Fetcher> mFetcher = nullptr;


};

#endif // WAITINGWIDGET_H
