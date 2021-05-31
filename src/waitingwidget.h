#ifndef WAITINGWIDGET_H
#define WAITINGWIDGET_H

#include <vector>

#include <QWidget>
#include <QListWidgetItem>

#include "waitinglistwidgetitem.h"
#include "waitinglistwidgetitemedit.h"

namespace Ui {
class WaitingWidget;
}

class WaitingWidget : public QWidget
{
    Q_OBJECT
signals:
    void addListButtonReleased();

public:
    explicit WaitingWidget(QWidget *parent = nullptr);
    ~WaitingWidget();

private slots:
    void onDeleteButtonReleased();
    void onAddListButtonReleased();
    void onLoadListsButtonReleased();
    void onLaunchButtonReleased();
    void onWaitingListWidgetItemEditSaved();

private:
    Ui::WaitingWidget *ui;

    std::vector<WaitingListWidgetItem *> mWaitingListItemVector;
};

#endif // WAITINGWIDGET_H
