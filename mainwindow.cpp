#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QWidget>
#include <QFrame>
#include <QGridLayout>
#include <QPushButton>
#include <QPainter>
#include <QEventLoop>
#include <QLabel>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mRepository = std::make_shared<Repository>();
    mFetcher = std::make_shared<Fetcher>();
    mGroupTableEdit = std::make_shared<GroupTableEdit>(this);
    mSendingProgress = std::make_shared<SendingProgress>(this);

    mFetcher->setRepository(mRepository);

    mGroupTableEdit->setFetcher(mFetcher);
    mGroupTableEdit->setRepository(mRepository);

    mSendingProgress->setFetcher(mFetcher);
    mSendingProgress->setRepository(mRepository);

    connect(ui->editListButton, &QPushButton::released,
            this, &MainWindow::onEditListButtonReleased);

    connect(mRepository.get(), &Repository::groupDataUpdated,
            this, &MainWindow::onGroupDataUpdated);

    connect(ui->sendButton, &QPushButton::released,
            this, &MainWindow::onSendButtonReleased);

    connect(this, &MainWindow::messageSent,
            mSendingProgress.get(), &SendingProgress::onMessageSent);

    connect(ui->addNewPhotoButton, &QPushButton::released,
            this, &MainWindow::onAddNewPhotoButtonReleased);

    connect(ui->deleteSelectedPhotosButton, &QPushButton::released,
            this, &MainWindow::onDeleteSelectedPhotosReleased);

    connect(ui->photosListWidget, &QListWidget::doubleClicked,
            this, &MainWindow::onPhotosListWidgetDoubleClicked);
}

void MainWindow::onAddNewPhotoButtonReleased()
{
    if (ui->photosListWidget->count() >= 6)
    {
        QMessageBox::warning(this, tr("VK Sender"), tr("Загрузить можно только 6 фотографий."));
        return;
    }

    QString name = QFileDialog::getOpenFileName(0, "Открыть", "",
                                                tr("Image Files (*.png *.jpg *.jpeg *.gif);;"
                                                   "All files (*.*)"));
    if (mPhotoPaths.find(name) != mPhotoPaths.end())
    {
        QMessageBox::warning(this, tr("VK Sender"), tr("Данное изображение уже добавлено в список."));
        return;
    }

    QListWidgetItem* listItem = new QListWidgetItem();
    listItem->setIcon(QPixmap(name));
    ui->photosListWidget->addItem(listItem);
    mPhotoPaths[name] = listItem;
}

void MainWindow::onDeleteSelectedPhotosReleased()
{
    qDebug() << "delete selected photos";
    QList<QListWidgetItem *> selectedItems = ui->photosListWidget->selectedItems();
    for (QListWidgetItem *item : selectedItems)
        for (auto it = mPhotoPaths.begin(); it != mPhotoPaths.end(); it++)
            if (it->second == item)
            {
                delete item;
                mPhotoPaths.erase(it);
            }
}

void MainWindow::onPhotosListWidgetDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    if (QListWidget *photosListWidget = dynamic_cast <QListWidget *>(sender()))
        if (QListWidgetItem *photoItem = photosListWidget->takeItem(index.row()))
            for (auto it = mPhotoPaths.begin(); it != mPhotoPaths.end(); it++)
                if (it->second == photoItem)
                {
                    delete photoItem;
                    mPhotoPaths.erase(it);
                }
}

void MainWindow::onSendButtonReleased()
{
    qDebug() << "on send button released";
    std::vector<Path> photoPaths;
    for (auto it = mPhotoPaths.begin(); it != mPhotoPaths.end(); it++)
        photoPaths.push_back(it->first);

    if (ui->messageEdit->toPlainText().isEmpty() && photoPaths.empty())
    {
        qDebug() << ui->messageEdit->toPlainText();
        QMessageBox::warning(this, tr("VK Sender"), tr("Напишите сообщение или добавьте изображения."));
        return;
    }

    mSendingProgress->open();
    emit messageSent(ui->messageEdit->toPlainText(), photoPaths);
}

void MainWindow::onGroupDataUpdated()
{
    ui->groupList->clear();

    for (const auto &group : mRepository->getGroupData())
    {
        QFrame *groupFrame = new QFrame(ui->groupList);
        groupFrame->setStyleSheet("border: 1px solid blue;"
                                  "background-color: gray;"
                                  "border-radius: 4px;");

        QLayout *groupFrameLayout = new QHBoxLayout();

        QLabel *vkLinkLabel = new QLabel(group.name);
        vkLinkLabel->setAlignment(Qt::AlignRight);
        vkLinkLabel->setStyleSheet("margin: 5px;"
                                   "color:blue;");

        groupFrameLayout->addWidget(vkLinkLabel);
        groupFrame->setLayout(groupFrameLayout);

        QListWidgetItem* item = new QListWidgetItem( ui->groupList );
        item->setSizeHint( groupFrame->sizeHint() );
        ui->groupList->setItemWidget( item, groupFrame );
    }
    qDebug() << "group data updated";

}

void MainWindow::onEditListButtonReleased()
{
    mGroupTableEdit->open();
}

MainWindow::~MainWindow()
{
    delete ui;
}

