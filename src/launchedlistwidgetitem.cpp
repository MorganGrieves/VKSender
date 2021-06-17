#include "launchedlistwidgetitem.h"
#include "ui_launchedlistwidgetitem.h"

LaunchedListWidgetItem::LaunchedListWidgetItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LaunchedListWidgetItem)
{
    ui->setupUi(this);

    ui->dateLabel->setText(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm"));
}

void LaunchedListWidgetItem::setFetcher(const std::shared_ptr<Fetcher> fetcher)
{
    mFetcher = fetcher;
}

void LaunchedListWidgetItem::setMessagePackAndLaunch(const MessagePack &message)
{
    connect(mFetcher.get(), &Fetcher::updatedPhoto,
            [&](QUuid id)
    {
        if (id == message.id)
        {
            ui->progressBar->setValue(ui->progressBar->value() + 1);
        }
    });

    connect(mFetcher.get(), &Fetcher::sentMessage,
            [&](QUuid id, Group group, PostNumber number)
    {
        if (id == message.id)
        {
            mResult.successfulGroups.push_back(QPair(group, number));
            mResult.errorGroups.erase(std::remove_if(mResult.errorGroups.begin(),
                                                     mResult.errorGroups.end(),
                                                     [&](Group errorGroup)
            {
                return (errorGroup.vkid == group.vkid);
            }));
            ui->progressBar->setValue(ui->progressBar->value() + 1);
        }
    });

    connect(mFetcher.get(), &Fetcher::sendingFinished,
            [&](QUuid id)
    {
        if (id == message.id)
        {
            ui->progressBar->setValue(mOperationsAmount);
            emit sendingFinished(mResult);
        }
    });

    std::function checkedGroups
    {
        [&]() -> size_t
        {
            size_t checkedGroups = 0;
            for (const auto &[group, checkState] : message.groups)
            {
                if (checkState == Qt::Checked)
                    checkedGroups++;
            }
            return checkedGroups;
        }
     };

    for (const auto &[group, checkState] : message.groups)
    {
        if (checkState == Qt::Checked)
            mResult.errorGroups.push_back(group);
    }

    mOperationsAmount = message.photoPaths.size() ? message.photoPaths.size() * checkedGroups() : checkedGroups();
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(mOperationsAmount);
    ui->topicLabel->setText(message.title);
    mResult.message = message;
    mFetcher->sendMessage(message);
}

LaunchedListWidgetItem::~LaunchedListWidgetItem()
{
    delete ui;
}
