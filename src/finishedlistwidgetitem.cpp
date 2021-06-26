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

    ui->sentGroupsLabel->setText("Разослано: " + QString::number(result.successfulGroups.count()));
    ui->notSentGroupsLabel->setText("Ошибок: " + QString::number(result.errorGroups.count()));
    ui->topicLabel->setText(result.message.title);

    connect(mFinishedEdit, &FinishedListWidgetItemEdit::backToWaiting,
            this, &FinishedListWidgetItem::backToWaiting);
    connect(mFinishedEdit, &FinishedListWidgetItemEdit::backButtonReleased,
            this, &FinishedListWidgetItem::onBackButtonReleased);
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
}

void FinishedListWidgetItem::onBackButtonReleased()
{
    emit showFinishedWidget();
}

FinishedListWidgetItem::~FinishedListWidgetItem()
{
    mFinishedEdit->deleteLater();
    delete ui;
}
