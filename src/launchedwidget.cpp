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
    ui->packCounterLabel->setText(
                QString::number(ui->listWidget->layout()->children().count()));
    connect(item, &LaunchedListWidgetItem::sendingFinished,
            [this, item](SendingResult result)
   {
        item->deleteLater();
        emit sendingFinished(result);
    });

    item->setMessagePackAndLaunch(pack);
}

LaunchedWidget::~LaunchedWidget()
{
    delete ui;
}
