#include "launchedwidget.h"
#include "ui_launchedwidget.h"

LaunchedWidget::LaunchedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LaunchedWidget)
{
    ui->setupUi(this);
}

void LaunchedWidget::setFetcher(const std::shared_ptr<Fetcher> fetcher)
{
    mFetcher = fetcher;
}

void LaunchedWidget::addLaunchedItem(const MessagePack &pack)
{
    LaunchedListWidgetItem *item = new LaunchedListWidgetItem(this);
    item->setFetcher(mFetcher);
    item->setMessagePackAndLaunch(pack);
    ui->listWidget->layout()->addWidget(item);
    ui->packCounterLabel->setText(
                QString::number(ui->listWidget->layout()->children().count()));
}

LaunchedWidget::~LaunchedWidget()
{
    delete ui;
}
