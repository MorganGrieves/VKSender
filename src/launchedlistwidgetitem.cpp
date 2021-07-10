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


    connect(mFetcher.get(), &Fetcher::updatedPhoto,
            this, &LaunchedListWidgetItem::onAttachmentUpdated);
    connect(mFetcher.get(), &Fetcher::updatedVideo,
            this, &LaunchedListWidgetItem::onAttachmentUpdated);
    connect(mFetcher.get(), &Fetcher::updatedAudio,
            this, &LaunchedListWidgetItem::onAttachmentUpdated);
    connect(mFetcher.get(), &Fetcher::updatedDoc,
            this, &LaunchedListWidgetItem::onAttachmentUpdated);

    connect(mFetcher.get(), &Fetcher::sentMessage,
            [this](QUuid id, Group group, PostNumber number)
    {
        if (id == mMessage.id)
        {
            QPair groupAndPost(group, number);
            mResult.successfulGroups.push_back(groupAndPost);
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
            [this](QUuid id)
    {
        if (id == mMessage.id)
        {
            ui->progressBar->setValue(mOperationsAmount);
            emit sendingFinished(mResult);
        }
    });
}

void LaunchedListWidgetItem::setMessagePackAndLaunch(const MessagePack &message)
{
    mMessage = message;

    auto checkedGroups =
        [&]() -> size_t
        {
            size_t checkedGroups = 0;
            for (const auto &[group, checkState] : message.groups)
                if (checkState == Qt::Checked)
                    checkedGroups++;
            return checkedGroups;
        };

    for (const auto &[group, checkState] : message.groups)
        if (checkState == Qt::Checked)
            mResult.errorGroups.push_back(group);

    mOperationsAmount = message.photoPaths.size() * checkedGroups()
                 + message.videoPaths.size()
                 + message.audioPaths.size()
                 + message.docPaths.size() * checkedGroups()
                 + checkedGroups();
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(mOperationsAmount);
    ui->topicLabel->setText(message.title);
    mResult.message = message;
    mFetcher->sendMessage(mMessage);
}

void LaunchedListWidgetItem::onAttachmentUpdated(QUuid id)
{
    if (id == mMessage.id)
        ui->progressBar->setValue(ui->progressBar->value() + 1);
}

LaunchedListWidgetItem::~LaunchedListWidgetItem()
{
    delete ui;
}
