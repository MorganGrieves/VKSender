#include "launchedwidget.h"
#include "ui_launchedwidget.h"

LaunchedWidget::LaunchedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LaunchedWidget)
{
    ui->setupUi(this);
}

LaunchedWidget::~LaunchedWidget()
{
    delete ui;
}
