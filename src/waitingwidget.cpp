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

void WaitingWidget::onDeleteButtonReleased()
{
    qDebug() << sender() << qobject_cast<WaitingListWidgetItem *>(sender());
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
    //ui->listWidget->updateGeometry();
}

void WaitingWidget::onAddListButtonReleased()
{
    WaitingListWidgetItem *item = new WaitingListWidgetItem();
    item->setFetcher(mFetcher);
    connect(item, &WaitingListWidgetItem::deleteButtonReleased,
            this, &WaitingWidget::onDeleteButtonReleased);
    connect(item, &WaitingListWidgetItem::launchButtonReleased,
            this, &WaitingWidget::onLaunchButtonReleased);
    connect(item, &WaitingListWidgetItem::waitingListWidgetItemReleased,
            this, &WaitingWidget::waitingListWidgetItemReleased);
    item->showItemEdit();
    ui->listWidget->layout()->addWidget(item);
}

void WaitingWidget::onLoadListsButtonReleased()
{
    //загрузка файла с ожидающими людьми
}

void WaitingWidget::onLaunchButtonReleased()
{
    //Желтая кнопка на пункт меню
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
