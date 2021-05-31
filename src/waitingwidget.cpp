#include "waitingwidget.h"
#include "ui_waitingwidget.h"

WaitingWidget::WaitingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaitingWidget)
{
    ui->setupUi(this);
}

WaitingWidget::~WaitingWidget()
{
    delete ui;
}
