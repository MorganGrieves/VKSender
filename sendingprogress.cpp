#include "sendingprogress.h"
#include "ui_sendingprogress.h"

SendingProgress::SendingProgress(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SendingProgress)
{
    ui->setupUi(this);
}

SendingProgress::~SendingProgress()
{
    delete ui;
}

void SendingProgress::setRepository(const std::shared_ptr<Repository> repository)
{
    mRepository = repository;
}

void SendingProgress::setFetcher(const std::shared_ptr<Fetcher> fetcher)
{
    mFetcher = fetcher;

    connect(mFetcher.get(), &Fetcher::updatedPhoto,
            [this](){
        ui->progressBar->setValue(ui->progressBar->value() + 1);
    });

    connect(mFetcher.get(), &Fetcher::sentMessage,
            [this](Group group, bool success){
        ui->progressBar->setValue(ui->progressBar->value() + 1);

        if (success)
            ui->resultLabel->setText(ui->resultLabel->text()
                                     + QString("<p style=\"font-weight: 600; color: green;\">"
                                               "Пост в %1 успешно опубликован."
                                               "</p>").arg(group.name));
        else
            ui->resultLabel->setText(ui->resultLabel->text()
                                     + QString("<p style=\"font-weight: 600; color: green;\">"
                                               "Пост в %1 не удалось опубликовать."
                                               "</p>").arg(group.name));
        sentMessages++;

        if (sentMessages == mRepository->getGroupData().size())\
            ui->resultLabel->setText(ui->resultLabel->text()
                                     + "<p>"
                                       "Рассылка завершена."
                                        "</p>");
    });


    connect(this, &SendingProgress::messageSent,
            mFetcher.get(), &Fetcher::onMessageSent);
}

void SendingProgress::onMessageSent(QString text, std::vector<Path> photoPaths)
{
    int operations = 2 * photoPaths.size() + mRepository->getGroupData().size();
    ui->progressBar->setMaximum(operations);
    ui->resultLabel->setText("");
    ui->progressBar->setValue(0);
    sentMessages = 0;
    emit messageSent(text, photoPaths);
}
