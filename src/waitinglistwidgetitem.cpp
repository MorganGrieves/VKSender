#include "waitinglistwidgetitem.h"
#include "ui_waitinglistwidgetitem.h"

WaitingListWidgetItem::WaitingListWidgetItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaitingListWidgetItem)
{
    ui->setupUi(this);
}

WaitingListWidgetItem::~WaitingListWidgetItem()
{
    delete ui;
}
