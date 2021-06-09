#include "waitinglistwidgetitem.h"
#include "ui_waitinglistwidgetitem.h"

#include <QListWidgetItem>

WaitingListWidgetItem::WaitingListWidgetItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaitingListWidgetItem)
{
    ui->setupUi(this);

    mEditItem = new WaitingListWidgetItemEdit();

    connect(ui->deleteButton, &QPushButton::released,
            this, &WaitingListWidgetItem::onDeleteButtonReleased);
    connect(ui->launchButton, &QPushButton::released,
            this, &WaitingListWidgetItem::onLaunchButtonReleased);
}

void WaitingListWidgetItem::showItemEdit()
{
    emit waitingListWidgetItemReleased(mEditItem);
}

void WaitingListWidgetItem::setFetcher(const std::shared_ptr<Fetcher> fetcher)
{
    mFetcher = fetcher;
    mEditItem->setFetcher(fetcher);
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

void WaitingListWidgetItem::onLaunchButtonReleased()
{
    emit launchButtonReleased();
}

WaitingListWidgetItem::~WaitingListWidgetItem()
{
    delete ui;
    delete mEditItem;
}
