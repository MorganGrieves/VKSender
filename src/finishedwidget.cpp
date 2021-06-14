#include "finishedwidget.h"
#include "ui_finishedwidget.h"

FinishedWidget::FinishedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FinishedWidget)
{
    ui->setupUi(this);
}

void FinishedWidget::setFetcher(const std::shared_ptr<Fetcher> fetcher)
{
    mFetcher = fetcher;
}

FinishedWidget::~FinishedWidget()
{
    delete ui;
}
