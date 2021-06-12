#ifndef LAUNCHEDLISTWIDGETITEM_H
#define LAUNCHEDLISTWIDGETITEM_H

#include <QWidget>

namespace Ui {
class LaunchedListWidgetItem;
}

class LaunchedListWidgetItem : public QWidget
{
    Q_OBJECT

public:
    explicit LaunchedListWidgetItem(QWidget *parent = nullptr);
    ~LaunchedListWidgetItem();

private:
    Ui::LaunchedListWidgetItem *ui;
};

#endif // LAUNCHEDLISTWIDGETITEM_H
