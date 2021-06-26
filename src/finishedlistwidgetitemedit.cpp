#include "finishedlistwidgetitemedit.h"
#include "ui_finishedlistwidgetitemedit.h"

FinishedListWidgetItemEdit::FinishedListWidgetItemEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FinishedListWidgetItemEdit)
{
    ui->setupUi(this);

    ui->noErrorGroupList->layout()->setAlignment(Qt::AlignTop);
    ui->errorGroupList->layout()->setAlignment(Qt::AlignTop);

    connect(ui->cancelButton, &QPushButton::released,
            this, &FinishedListWidgetItemEdit::onCancelButtonReleased);
    connect(ui->backToWaitingButton, &QPushButton::released,
            this, &FinishedListWidgetItemEdit::onBackToWaitingButtonReleased);
    connect(ui->backButton, &QPushButton::released,
            this, &FinishedListWidgetItemEdit::onBackButtonReleased);
}

void FinishedListWidgetItemEdit::setFetcher(const std::shared_ptr<Fetcher> fetcher)
{
    mFetcher = fetcher;
}

void FinishedListWidgetItemEdit::setSendingResult(SendingResult result)
{
    mResult = result;
    for (const auto& [group, postNumber] : result.successfulGroups)
    {
        NoErrorGroupItem *item = new NoErrorGroupItem();
        item->setFetcher(mFetcher);
        item->setGroup(group);
        item->setPostNumber(postNumber);
        connect(item, &NoErrorGroupItem::abortButtonReleased,
                [this](const Group &group)
        {
            mResult.successfulGroups.erase(
                        std::remove_if(mResult.successfulGroups.begin(),
                                       mResult.successfulGroups.end(),
                                       [group](QPair<Group, QString> &pair)
                                       { return pair.first.vkid == group.vkid; }));
        });
        ui->noErrorGroupList->layout()->addWidget(item);
    }

    for (const auto& group : result.errorGroups)
    {
        ErrorGroupItem *item = new ErrorGroupItem();
        item->setGroup(group);
        ui->errorGroupList->layout()->addWidget(item);
    }
}

void FinishedListWidgetItemEdit::onCancelButtonReleased()
{
    for (const auto &[group, postNumber] : mResult.successfulGroups)
        mFetcher->onPostDelete(postNumber, group.vkid);
    qDeleteAll(ui->noErrorGroupList->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly));
}

void FinishedListWidgetItemEdit::onBackToWaitingButtonReleased()
{
    emit backToWaiting(mResult.message);
}

void FinishedListWidgetItemEdit::onBackButtonReleased()
{
    emit backButtonReleased();
}

FinishedListWidgetItemEdit::~FinishedListWidgetItemEdit()
{
    delete ui;
}
