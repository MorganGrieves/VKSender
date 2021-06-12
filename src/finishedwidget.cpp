#include "finishedwidget.h"
#include "ui_finishedwidget.h"

FinishedWidget::FinishedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FinishedWidget)
{
    ui->setupUi(this);
}

FinishedWidget::~FinishedWidget()
{
    delete ui;
}
