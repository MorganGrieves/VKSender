#ifndef WAITINGLISTWIDGETITEM_H
#define WAITINGLISTWIDGETITEM_H

#include <QWidget>

namespace Ui {
class WaitingListWidgetItem;
}

class WaitingListWidgetItem : public QWidget
{
    Q_OBJECT

public:
    explicit WaitingListWidgetItem(QWidget *parent = nullptr);
    ~WaitingListWidgetItem();

private:
    Ui::WaitingListWidgetItem *ui;
};

#endif // WAITINGLISTWIDGETITEM_H
