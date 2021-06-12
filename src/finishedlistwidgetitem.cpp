#include "finishedlistwidgetitem.h"
#include "ui_finishedlistwidgetitem.h"

FinishedListWidgetItem::FinishedListWidgetItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FinishedListWidgetItem)
{
    ui->setupUi(this);
}

FinishedListWidgetItem::~FinishedListWidgetItem()
{
    delete ui;
}
