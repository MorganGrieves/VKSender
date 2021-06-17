#include "finishedlistwidgetitem.h"
#include "ui_finishedlistwidgetitem.h"

FinishedListWidgetItem::FinishedListWidgetItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FinishedListWidgetItem)
{
    ui->setupUi(this);

    ui->dateLabel->setText(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm"));

    connect(ui->backToWaitingButton, &QPushButton::released,
            this, &FinishedListWidgetItem::onBackToWaitingButtonReleased);
    mFinishedEdit = new FinishedListWidgetItemEdit();
}

void FinishedListWidgetItem::setFetcher(const std::shared_ptr<Fetcher> fetcher)
{
    mFetcher = fetcher;
}

void FinishedListWidgetItem::setSendingResult(SendingResult result)
{
    mSendingResult = result;
    mFinishedEdit->setFetcher(mFetcher);
    mFinishedEdit->setSendingResult(result);
    connect(mFinishedEdit, &FinishedListWidgetItemEdit::backToWaiting,
            this, &FinishedListWidgetItem::backToWaiting);
}

void FinishedListWidgetItem::showItemEdit()
{
    emit finishedListWidgetItemReleased(mFinishedEdit);
}

void FinishedListWidgetItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        showItemEdit();
}

void FinishedListWidgetItem::onBackToWaitingButtonReleased()
{
    emit backToWaiting(mSendingResult.message);
    this->deleteLater();
}

FinishedListWidgetItem::~FinishedListWidgetItem()
{
    delete ui;
}
