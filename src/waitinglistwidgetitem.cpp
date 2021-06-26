#include "waitinglistwidgetitem.h"
#include "ui_waitinglistwidgetitem.h"

#include <QListWidgetItem>

WaitingListWidgetItem::WaitingListWidgetItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaitingListWidgetItem)
{
    ui->setupUi(this);

    mEditItem = new WaitingListWidgetItemEdit();

    ui->dateLabel->setText(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm"));

    connect(ui->deleteButton, &QPushButton::released,
            this, &WaitingListWidgetItem::onDeleteButtonReleased);
}

void WaitingListWidgetItem::showItemEdit()
{
    mTmpEditItem = new WaitingListWidgetItemEdit(*mEditItem);
    connect(mTmpEditItem, &WaitingListWidgetItemEdit::saveButtonReleased,
            this, &WaitingListWidgetItem::onSaveButtonReleased);
    connect(mTmpEditItem, &WaitingListWidgetItemEdit::cancelButtonReleased,
            this, &WaitingListWidgetItem::onCancelButtonReleased);
    connect(mTmpEditItem, &WaitingListWidgetItemEdit::launchButtonReleased,
            this, &WaitingListWidgetItem::onLaunchButtonReleased);
    connect(mTmpEditItem, &WaitingListWidgetItemEdit::backButtonReleased,
            this, &WaitingListWidgetItem::onBackButtonReleased);
    emit waitingListWidgetItemReleased(mTmpEditItem);
}

void WaitingListWidgetItem::setFetcher(const std::shared_ptr<Fetcher> fetcher)
{
    mFetcher = fetcher;
    mEditItem->setFetcher(fetcher);
}

void WaitingListWidgetItem::setMessagePack(MessagePack message)
{
    mEditItem->setMessagePack(message);

    ui->dateLabel->setText(message.dateCreation);
    ui->topicLabel->setText(message.title);

}

MessagePack WaitingListWidgetItem::getMessagePack() const
{
    MessagePack message = mEditItem->getMessageInfo();
    message.dateCreation = ui->dateLabel->text();
    return message;
}

void WaitingListWidgetItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        showItemEdit();
}

void WaitingListWidgetItem::onDeleteButtonReleased()
{
    emit deleteButtonReleased();
}

void WaitingListWidgetItem::onSaveButtonReleased()
{
    mEditItem->deleteLater();
    mEditItem = mTmpEditItem;

    if (!mEditItem->getPackName().isEmpty())
        ui->topicLabel->setText(mEditItem->getPackName());
    else
        ui->topicLabel->setText("Нет названия");
    ui->receiverLabel->setText("Получатели: " + QString::number(mEditItem->getCheckedGroupsNumber()));
}

void WaitingListWidgetItem::onCancelButtonReleased()
{
    mTmpEditItem->deleteLater();
    showItemEdit();
}

void WaitingListWidgetItem::onLaunchButtonReleased()
{
    if (WaitingListWidgetItemEdit *itemEdit
            = qobject_cast<WaitingListWidgetItemEdit *>(sender()))
        emit launchSending(itemEdit->getMessageInfo());
}

void WaitingListWidgetItem::onBackButtonReleased()
{
    ui->receiverLabel->setText("Получатели: " + QString::number(mEditItem->getCheckedGroupsNumber()));

    emit showWaitingWidget();
}

WaitingListWidgetItem::~WaitingListWidgetItem()
{
    delete ui;
    mEditItem->deleteLater();
}
