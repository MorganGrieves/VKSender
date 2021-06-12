#ifndef FINISHEDLISTWIDGETITEM_H
#define FINISHEDLISTWIDGETITEM_H

#include <QWidget>

namespace Ui {
class FinishedListWidgetItem;
}

class FinishedListWidgetItem : public QWidget
{
    Q_OBJECT

public:
    explicit FinishedListWidgetItem(QWidget *parent = nullptr);
    ~FinishedListWidgetItem();

private:
    Ui::FinishedListWidgetItem *ui;
};

#endif // FINISHEDLISTWIDGETITEM_H
