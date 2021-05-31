#ifndef WAITINGLISTWIDGETITEMEDIT_H
#define WAITINGLISTWIDGETITEMEDIT_H

#include <QWidget>

namespace Ui {
class WaitingListWidgetItemEdit;
}

class WaitingListWidgetItemEdit : public QWidget
{
    Q_OBJECT

public:
    explicit WaitingListWidgetItemEdit(QWidget *parent = nullptr);
    ~WaitingListWidgetItemEdit();

private:
    Ui::WaitingListWidgetItemEdit *ui;
};

#endif // WAITINGLISTWIDGETITEMEDIT_H
