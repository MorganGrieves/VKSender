#include "launchedlistwidgetitem.h"
#include "ui_launchedlistwidgetitem.h"

LaunchedListWidgetItem::LaunchedListWidgetItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LaunchedListWidgetItem)
{
    ui->setupUi(this);
}

LaunchedListWidgetItem::~LaunchedListWidgetItem()
{
    delete ui;
}
