#include "waitingwidget.h"
#include "ui_waitingwidget.h"

#include <QWidget>
#include <QSizePolicy>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QDebug>

WaitingWidget::WaitingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaitingWidget)
{
    ui->setupUi(this);

    connect(ui->addListButton, &QPushButton::released,
            this, &WaitingWidget::onAddListButtonReleased);

    ui->listWidget->layout()->setAlignment(Qt::AlignTop);
}

void WaitingWidget::setFetcher(const std::shared_ptr<Fetcher> fetcher)
{
    mFetcher = fetcher;
}

void WaitingWidget::addListItem(MessagePack message)
{
    WaitingListWidgetItem *item = new WaitingListWidgetItem(this);
    item->setFetcher(mFetcher);
    item->setMessagePack(message);
    connect(item, &WaitingListWidgetItem::deleteButtonReleased,
            this, &WaitingWidget::onDeleteButtonReleased);
    connect(item, &WaitingListWidgetItem::waitingListWidgetItemReleased,
            this, &WaitingWidget::waitingListWidgetItemReleased);
    connect(item, &WaitingListWidgetItem::launchSending,
            this, &WaitingWidget::onLaunchSending);
    connect(item, &WaitingListWidgetItem::showWaitingWidget,
            [&]()
    {
        emit showWidget();
    });

    item->showItemEdit();
    ui->listWidget->layout()->addWidget(item);
    ui->packCounterLabel->setText(
                QString::number(ui->listWidget->layout()->children().count()));

}

void WaitingWidget::onDeleteButtonReleased()
{
//    std::vector<WaitingListWidgetItem *>::iterator widgetElement = std::find(mWaitingList.begin(),
//                                                            mWaitingList.end(),
//                                                            qobject_cast<WaitingListWidgetItem *>(sender()));

//    if (widgetElement != mWaitingList.end())
//        std::for_each(widgetElement + 1, mWaitingList.end(),
//                      [](WaitingListWidgetItem x)
//        {

//        });

    sender()->deleteLater();
    mWaitingListItemVector.erase(std::remove(mWaitingListItemVector.begin(),
                                             mWaitingListItemVector.end(),
                                             sender()),
                                 mWaitingListItemVector.end());
    ui->packCounterLabel->setText(
                QString::number(ui->listWidget->layout()->children().count()));
}

void WaitingWidget::onAddListButtonReleased()
{
    WaitingListWidgetItem *item = new WaitingListWidgetItem(this);
    item->setFetcher(mFetcher);
    connect(item, &WaitingListWidgetItem::deleteButtonReleased,
            this, &WaitingWidget::onDeleteButtonReleased);
    connect(item, &WaitingListWidgetItem::waitingListWidgetItemReleased,
            this, &WaitingWidget::waitingListWidgetItemReleased);
    connect(item, &WaitingListWidgetItem::launchSending,
            this, &WaitingWidget::onLaunchSending);
    connect(item, &WaitingListWidgetItem::showWaitingWidget,
            [&]()
    {
        emit showWidget();
    });

    item->showItemEdit();
    ui->listWidget->layout()->addWidget(item);
    ui->packCounterLabel->setText(
                QString::number(ui->listWidget->layout()->children().count()));
}

void WaitingWidget::onLoadListsButtonReleased()
{
    //загрузка файла с ожидающими людьми
}

void WaitingWidget::onLaunchSending(MessagePack message)
{
    if (WaitingListWidgetItem *item = qobject_cast<WaitingListWidgetItem *>(sender()))
        item->deleteLater();
    emit launchSending(message);
}

void WaitingWidget::onWaitingListWidgetItemEditSaved()
{
    WaitingListWidgetItem *senderItem = qobject_cast<WaitingListWidgetItem *>(sender());

    if (senderItem == nullptr)
    {
        qDebug() << "senderItem" << senderItem;
        return;
    }
}

WaitingWidget::~WaitingWidget()
{
    delete ui;
}
