#ifndef WAITINGLISTWIDGETITEM_H
#define WAITINGLISTWIDGETITEM_H

#include <QWidget>

#include "waitinglistwidgetitemedit.h"

namespace Ui {
class WaitingListWidgetItem;
}

class WaitingListWidgetItem : public QWidget
{
    Q_OBJECT

signals:
    void deleteButtonReleased();
    void launchButtonReleased();

public:
    explicit WaitingListWidgetItem(QWidget *parent = nullptr);
    ~WaitingListWidgetItem();

    //void setWaitingListWidgetItemEdit(const WaitingListWidgetItemEdit &item);
    //WaitingListWidgetItemEdit &getWaitingListWidgetItemEdit() const;

private slots:
    void onDeleteButtonReleased();
    void onLaunchButtonReleased();

private:
    Ui::WaitingListWidgetItem *ui;

    WaitingListWidgetItemEdit *mEditItem = nullptr;

};

#endif // WAITINGLISTWIDGETITEM_H
