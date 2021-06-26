#include "launchedwidget.h"
#include "ui_launchedwidget.h"

LaunchedWidget::LaunchedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LaunchedWidget)
{
    ui->setupUi(this);

    ui->listWidget->layout()->setAlignment(Qt::AlignTop);

}

void LaunchedWidget::setFetcher(const std::shared_ptr<Fetcher> fetcher)
{
    mFetcher = fetcher;
}

void LaunchedWidget::addLaunchedItem(const MessagePack &pack)
{
    LaunchedListWidgetItem *item = new LaunchedListWidgetItem(this);
    item->setFetcher(mFetcher);
    ui->listWidget->layout()->addWidget(item);
    ui->packCounterLabel->setNum(listSize());
    connect(item, &LaunchedListWidgetItem::sendingFinished,
            [this, item](SendingResult result)
   {
        delete item;
        ui->listWidget->layout()->update();
        ui->packCounterLabel->setNum(listSize());
        emit sendingFinished(result);
    });

    item->setMessagePackAndLaunch(pack);
}

int LaunchedWidget::listSize() const
{
    return ui->listWidget->layout()->count();
}

LaunchedWidget::~LaunchedWidget()
{
    delete ui;
}
