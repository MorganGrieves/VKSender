#include "waitinglistwidgetitem.h"
#include "ui_waitinglistwidgetitem.h"

#include <QListWidgetItem>

WaitingListWidgetItem::WaitingListWidgetItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaitingListWidgetItem)
{
    ui->setupUi(this);

    connect(ui->deleteButton, &QPushButton::released,
            this, &WaitingListWidgetItem::onDeleteButtonReleased);
    connect(ui->launchButton, &QPushButton::released,
            this, &WaitingListWidgetItem::onLaunchButtonReleased);
}

//void WaitingListWidgetItem::setWaitingListWidgetItemEdit(const WaitingListWidgetItemEdit &item)
//{
//    mEditItem = item;
//}

//WaitingListWidgetItemEdit &WaitingListWidgetItem::getWaitingListWidgetItemEdit() const
//{
//    return mEditItem;
//}

void WaitingListWidgetItem::onDeleteButtonReleased()
{
    emit deleteButtonReleased();
}

void WaitingListWidgetItem::onLaunchButtonReleased()
{
    emit launchButtonReleased();
}

WaitingListWidgetItem::~WaitingListWidgetItem()
{
    delete ui;
}
