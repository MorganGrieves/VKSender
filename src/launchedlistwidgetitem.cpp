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
            [&](QUuid id, Group group)
    {
        if (id == message.id)
        {
            mResult.successfulGroups.push_back(QPair(group, 1));
            ui->progressBar->setValue(ui->progressBar->value() + 1);
        }
    });
    connect(mFetcher.get(), &Fetcher::sendingFinished,
            [&](QUuid id)
    {
        if (id == message.id)
        {
            ui->progressBar->setValue(mOperationsAmount);
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

    mOperationsAmount = message.photoPaths.size() * checkedGroups();
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
