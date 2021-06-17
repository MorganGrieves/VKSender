#include "finishedwidget.h"
#include "ui_finishedwidget.h"

FinishedWidget::FinishedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FinishedWidget)
{
    ui->setupUi(this);

    ui->listWidget->layout()->setAlignment(Qt::AlignTop);
}

void FinishedWidget::setFetcher(const std::shared_ptr<Fetcher> fetcher)
{
    mFetcher = fetcher;
}

void FinishedWidget::addFinishedItem(const SendingResult &result)
{
    FinishedListWidgetItem *item = new FinishedListWidgetItem(this);
    connect(item, &FinishedListWidgetItem::finishedListWidgetItemReleased,
            this, &FinishedWidget::finishedListWidgetItemReleased);
    connect(item, &FinishedListWidgetItem::backToWaiting,
            this, &FinishedWidget::abortionFinished);
    ui->listWidget->layout()->addWidget(item);
    item->setFetcher(mFetcher);
    item->setSendingResult(result);
}

FinishedWidget::~FinishedWidget()
{
    delete ui;
}
