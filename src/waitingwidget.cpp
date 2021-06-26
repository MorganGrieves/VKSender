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
    connect(ui->loadListsButton, &QPushButton::released,
            this, &WaitingWidget::onLoadListsButtonReleased);
    connect(ui->saveListsButton, &QPushButton::released,
            this, &WaitingWidget::onSaveListsButtonReleased);

    ui->listWidget->layout()->setAlignment(Qt::AlignTop);
}

void WaitingWidget::setFetcher(const std::shared_ptr<Fetcher> fetcher)
{
    mFetcher = fetcher;
}

void WaitingWidget::setRepository(const std::shared_ptr<Repository> repository)
{
    mRepository = repository;
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
    ui->packCounterLabel->setNum(listSize());
}

int WaitingWidget::listSize() const
{
    return ui->listWidget->layout()->count();
}

void WaitingWidget::loadLists()
{
    if (listSize())
    {
        QMessageBox::StandardButton reply =
                QMessageBox::question(this, "VKSender", "Сохранить список групп?",
                                      QMessageBox::Yes|QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            QString saveFileName = QFileDialog::getSaveFileName(0, "Сохранить", "",
                                                        tr("VKSender Files (*.vks)"));
            if (!saveFileName.isEmpty())
                mRepository->serialize(getAllMessagePacks(), saveFileName);
        }
    }

    QString openFileName = QFileDialog::getOpenFileName(0, "Открыть", "",
                                                tr("VKSender Files (*.vks)"));
    if (!openFileName.isEmpty())
    {
        qDeleteAll(ui->listWidget->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly));
        setAllMessagePacks(mRepository->deserialize(openFileName));
    }
}

void WaitingWidget::saveLists()
{
    if (listSize())
    {
        QString saveFileName = QFileDialog::getSaveFileName(0, "Сохранить", "",
                                                    tr("VKSender Files (*.vks)"));
        if (!saveFileName.isEmpty())
            mRepository->serialize(getAllMessagePacks(), saveFileName);
    } else
        QMessageBox::warning(0, tr("VK Sender"), tr("Вы не создали элемента рассылки!"));
}

QVector<MessagePack> WaitingWidget::getAllMessagePacks() const
{
    QVector<MessagePack> res;

    const auto links = ui->listWidget->findChildren<WaitingListWidgetItem *>();
    for(const auto & link : links)
        res.push_back(link->getMessagePack());

    return res;
}

void WaitingWidget::setAllMessagePacks(const QVector<MessagePack> &messages)
{
    for (const auto &message : messages)
        addListItem(message);
}

void WaitingWidget::onLoadListsButtonReleased()
{
    emit loadListsButtonRelease();
}

void WaitingWidget::onSaveListsButtonReleased()
{
    emit saveListsButtonRelease();
}

void WaitingWidget::onDeleteButtonReleased()
{
    WaitingListWidgetItem *item = qobject_cast<WaitingListWidgetItem *>(sender());
    if (item != nullptr)
    {
        delete sender();
        ui->listWidget->layout()->update();
    }
    int c = listSize();
    ui->packCounterLabel->setNum(c);
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
    ui->packCounterLabel->setNum(listSize());
}

void WaitingWidget::onLaunchSending(MessagePack message)
{
    delete sender();

    ui->listWidget->layout()->update();
    ui->packCounterLabel->setNum(listSize());
    emit launchSending(message);
}

WaitingWidget::~WaitingWidget()
{
    delete ui;
}
