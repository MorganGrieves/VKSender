#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QWidget>
#include <QFrame>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QAction>
#include <QPainterPath>
#include <QSettings>
#include <QWebView>

InterfaceListDelegate::InterfaceListDelegate(QObject *parent)
{}

void InterfaceListDelegate::paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex & index ) const
{
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::white);
    painter->translate(10, 0);

    QVariant isSeparator = index.data(Qt::UserRole + 1);

    if (isSeparator.isValid() && isSeparator.toBool())
    {
        QRect rct = option.rect;
        rct.setHeight(1);
        painter->fillRect(rct, QColor::fromRgb(qRgb(237, 238, 240)));

        painter->restore();
        return;
    }

    QRect r = option.rect;

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRoundedRect(r, 8, 8);
    path.addRect(r.left() + r.width() / 2, r.top(), r.width() / 2, r.height());

    if (option.state & QStyle::State_Selected)
    {
        QLinearGradient gradientSelected(r.x(), r.height() / 2 +  r.top(),
                                         r.x() + r.width(), r.height() / 2 + r.top());
        gradientSelected.setColorAt(0.0, QColor::fromRgb(101, 143, 192));
        gradientSelected.setColorAt(0.5, QColor::fromRgb(0, 212, 255));
        gradientSelected.setColorAt(1, QColor::fromRgb(0, 212, 255));
        painter->setBrush(gradientSelected);
        painter->fillPath(path.simplified(), gradientSelected);
    }
    else
    {
        QLinearGradient gradientSelected(r.x(), r.height() / 2 +  r.top(),
                                         r.x() + r.width(), r.height() / 2 + r.top());
        gradientSelected.setColorAt(0.0, QColor::fromRgb(255, 255, 255));
        gradientSelected.setColorAt(0.5, QColor::fromRgb(0, 212, 255));
        gradientSelected.setColorAt(1, QColor::fromRgb(0, 212, 255));
        painter->setBrush(gradientSelected);
        painter->fillPath(path.simplified(), gradientSelected);
    }

    QIcon ic = QIcon(qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
    QString title = index.data(Qt::DisplayRole).toString();

    int imageSpace = 0;
    if (!ic.isNull())
    {
        r = option.rect.adjusted(20, 15, -10, -10);
        ic.paint(painter, r, Qt::AlignVCenter|Qt::AlignLeft);
        imageSpace = 80;
    }

    r = option.rect.adjusted(imageSpace, 0, -15, -15);
    painter->setFont(QFont("Roboto", 10, QFont::Normal));
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignBottom|Qt::AlignLeft, title, &r);

    painter->restore();
}

QSize InterfaceListDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QVariant isSeparator = index.data(Qt::UserRole + 1);

    if (isSeparator.isValid() && isSeparator.toBool())
        return QSize(100, 1);

    return QSize(100, 45);
}

InterfaceListDelegate::~InterfaceListDelegate()
{}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mVkAuthorizationView = new QWebView();

    mRepository = std::make_shared<Repository>();
    mFetcher = std::make_shared<Fetcher>();
    mGroupTableEdit = std::make_shared<GroupTableEdit>(this);
    mSendingProgress = std::make_shared<SendingProgress>(this);

    if (mFetcher->tokenIsEmpty())
    {
        mGreetingWidget = std::make_shared<GreetingWidget>(this);
        //mGreetingWidget->move(rect().center() - mGreetingWidget->rect().center());
        mGreetingWidget->resize(rect().size());
        mGreetingWidget->show();
        connect(mGreetingWidget.get(), &GreetingWidget::accessTokenReceived,
                [this](QString token)
        {
            mGreetingWidget->close();
            mFetcher->setAccessToken(token);
        });
    }

    mFetcher->setRepository(mRepository);

    mGroupTableEdit->setFetcher(mFetcher);
    mGroupTableEdit->setRepository(mRepository);

    mSendingProgress->setFetcher(mFetcher);
    mSendingProgress->setRepository(mRepository);

    //get last date entrance and set to the label
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    if (settings.contains("Mainwindow/lastEntranceSecs"))
    {
        QDateTime lastEntrance =
                QDateTime::fromSecsSinceEpoch(settings.value("Mainwindow/lastEntranceSecs", 0).toULongLong());
        QLocale locale = QLocale::Russian;

        qDebug() << QDateTime::currentDateTime();

        QString labelText = "последний запуск ";

        if (QDateTime::currentDateTime().date() != lastEntrance.date())
            labelText += QString::number(lastEntrance.date().day()) + " "
                    + locale.toString(lastEntrance.date(), "MMMM") + " ";
        labelText += "в " + lastEntrance.toString("HH:mm") + " ";
        qDebug() << labelText;
        ui->lastEntranceLabel->setText(labelText);
    }

    setInterfaceListWidget();
    setTabs();
    hideAllTabs();
    mNothingHereWidget->show();

    mWaitingWidget->setFetcher(mFetcher);

    connect(ui->interfaceListWidget, &QListWidget::currentItemChanged,
            this, &MainWindow::onInterfaceListWidgetItemChanged);

    connect(mWaitingWidget, &WaitingWidget::waitingListWidgetItemReleased,
            [this](WaitingListWidgetItemEdit *item)
    {
        hideAllTabs();
        ui->tabFrame->layout()->addWidget(item);
    });

    connect(mFetcher.get(), &Fetcher::userPhoto100Update,
            this, &MainWindow::onProfilePictureUpdated);

    connect(mFetcher.get(), &Fetcher::userNameUpdate,
            this, &MainWindow::onUserNameUpdated);

    connect(ui->changeAccountButton, &QPushButton::released,
            this, &MainWindow::onChangeAccountButtonReleased);

    connect(mVkAuthorizationView, &QWebView::urlChanged,
            [&](const QUrl &url)
    {
        qDebug() << "View Url Changed" << url.url();
        QRegExp reg("access_token=[\\d\\w]+");
        if (reg.indexIn(url.fragment()) != -1)
        {
            QString exm = reg.cap(0);
            QString token = reg.cap(0).right(reg.cap(0).count() - reg.cap(0).indexOf("=") - 1);
            mFetcher->setAccessToken(token);
            mVkAuthorizationView->close();
            mFetcher->onUserDataUpdate();
        }
    });


//    connect(ui->openAction, &QAction::triggered,
//            [this](bool)
//    {
//        QMessageBox::StandardButton reply;
//        reply = QMessageBox::question(this, "VKSender", "Сохранить список групп?",
//                                      QMessageBox::Yes|QMessageBox::No);
//        if (reply == QMessageBox::Yes)
//        {
//            QString saveFileName = QFileDialog::getSaveFileName(0, "Сохранить", "",
//                                                        tr("VKSender Files (*.vks)"));
//            if (!saveFileName.isEmpty())
//                mRepository->serialize(saveFileName);

//            QString openFileName = QFileDialog::getOpenFileName(0, "Открыть", "",
//                                                        tr("VKSender Files (*.vks)"));
//            if (!openFileName.isEmpty())
//                mRepository->deserialize(openFileName);
//        }
//        else
//        {
//            QString openFileName = QFileDialog::getOpenFileName(0, "Открыть", "",
//                                                        tr("VKSender Files (*.vks)"));
//            if (!openFileName.isEmpty())
//                mRepository->deserialize(openFileName);
//        }
//    });

//    connect(ui->saveAction, &QAction::triggered,
//            [this](bool)
//    {
//        QString saveFileName = QFileDialog::getSaveFileName(0, "Сохранить", "",
//                                                    tr("VKSender Files (*.vks)"));
//        if (!saveFileName.isEmpty())
//            mRepository->serialize(saveFileName);
//    });

//    connect(ui->exitAction, &QAction::triggered,
//            [this](bool)
//    {
//        qApp->quit();
//    });

}

MainWindow::~MainWindow()
{
    delete ui;
    delete mVkAuthorizationView;

    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    settings.setValue("Mainwindow/lastEntranceSecs", QString::number(QDateTime::currentDateTime().toSecsSinceEpoch()));
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    mGreetingWidget->resize(rect().size());
}

void MainWindow::onProfilePictureUpdated()
{
    QPixmap photo = mFetcher->getUserPhoto100();
    photo = photo.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    ui->profilePictureLabel->setPixmap(roundPhoto100(photo));
}

void MainWindow::onUserNameUpdated()
{
    ui->accountNameLabel->setText(mFetcher->getUserName());
}

void MainWindow::onInterfaceListWidgetItemChanged(QListWidgetItem *current,
                                                  QListWidgetItem *previous)
{
    qDebug() << ui->tabFrame->children().count();
    switch (current->data(Qt::UserRole).toInt())
    {
    case Waiting:
        hideAllTabs();
        mWaitingWidget->show();
        break;
    case Finished:

        break;
    case Launched:
        //ui->nothingHereFrame->setHidden(true);
        break;
    case Settings:
        //ui->nothingHereFrame->setHidden(true);
        break;
    }
}

void MainWindow::onAddNewPhotoButtonReleased()
{
//    if (ui->photosListWidget->count() >= 6)
//    {
//        QMessageBox::warning(this, tr("VK Sender"), tr("Загрузить можно только 6 фотографий."));
//        return;
//    }

//    QString openImageName = QFileDialog::getOpenFileName(0, "Открыть", "",
//                                                tr("Image Files (*.png *.jpg *.jpeg *.gif);;"
//                                                   "All files (*.*)"));
//    if (openImageName.isEmpty())
//        return;

//    if (mPhotoPaths.find(openImageName) != mPhotoPaths.end())
//    {
//        QMessageBox::warning(this, tr("VK Sender"), tr("Данное изображение уже добавлено в список."));
//        return;
//    }

//    QListWidgetItem* listItem = new QListWidgetItem(ui->photosListWidget);
//    listItem->setIcon(QPixmap(openImageName));
//    mPhotoPaths[openImageName] = listItem;
//    qDebug() << "A new photo was added. mPhotoPaths.size =" << mPhotoPaths.size();
}

void MainWindow::onDeleteSelectedPhotosReleased()
{
//    qDebug() << "delete selected photos";
//    QList<QListWidgetItem *> selectedItems = ui->photosListWidget->selectedItems();
//    for (QListWidgetItem *item : selectedItems)
//        for (auto it = mPhotoPaths.begin(); it != mPhotoPaths.end();)
//            if (it->second == item)
//            {
//                delete item;
//                it = mPhotoPaths.erase(it);
//            }
//            else
//                it++;
//    qDebug() << "A photo was deleted. mPhotoPaths.size =" << mPhotoPaths.size();
//}

//void MainWindow::onPhotosListWidgetDoubleClicked(const QModelIndex& index)
//{
//    if (!index.isValid())
//        return;

//    if (QListWidget *photosListWidget = dynamic_cast <QListWidget *>(sender()))
//        if (QListWidgetItem *photoItem = photosListWidget->takeItem(index.row()))
//            for (auto it = mPhotoPaths.begin(); it != mPhotoPaths.end();)
//                if (it->second == photoItem)
//                {
//                    delete photoItem;
//                    it = mPhotoPaths.erase(it);
//                }
//                else
//                    it++;
//    qDebug() << "A photo was deleted. mPhotoPaths.size =" << mPhotoPaths.size();
}

void MainWindow::onSendButtonReleased()
{
//    qDebug() << "On send button released";
//    std::vector<Path> photoPaths;
//    for (auto it = mPhotoPaths.begin(); it != mPhotoPaths.end(); it++)
//        photoPaths.push_back(it->first);

//    if (ui->messageEdit->toPlainText().isEmpty() && photoPaths.empty())
//    {
//        QMessageBox::warning(this, tr("VK Sender"), tr("Напишите сообщение или добавьте изображения."));
//        return;
//    }

//    if (mRepository->getGroupData().isEmpty())
//    {
//        QMessageBox::warning(this, tr("VK Sender"), tr("Список групп пуст."));
//        return;
//    }

//    mSendingProgress->open();
//    emit messageSent(ui->messageEdit->toPlainText(), photoPaths);
}

void MainWindow::onGroupDataUpdated()
{
//    ui->groupList->clear();

//    for (const auto &group : mRepository->getGroupData())
//    {
//        QFrame *groupFrame = new QFrame(ui->groupList);
//        groupFrame->setStyleSheet("border: 1px solid blue;"
//                                  "background-color: gray;"
//                                  "border-radius: 4px;");

//        QLayout *groupFrameLayout = new QHBoxLayout(this);

//        QLabel *vkLinkLabel = new QLabel(group.name);
//        vkLinkLabel->setAlignment(Qt::AlignRight);
//        vkLinkLabel->setStyleSheet("margin: 5px;"
//                                   "color:blue;");

//        groupFrameLayout->addWidget(vkLinkLabel);
//        groupFrame->setLayout(groupFrameLayout);

//        QListWidgetItem* item = new QListWidgetItem( ui->groupList );
//        item->setSizeHint( groupFrame->sizeHint() );
//        ui->groupList->setItemWidget( item, groupFrame );
//    }
//    qDebug() << "group data updated";
}

void MainWindow::onEditListButtonReleased()
{
//    mGroupTableEdit->open();
//    qDebug() << "GroupTableEdit was opened";
}

void MainWindow::onChangeAccountButtonReleased()
{
    mVkAuthorizationView->load(QUrl("https://oauth.vk.com/authorize?"
                   "client_id=7656391"
                   "&display=page"
                   "&redirect_uri=https://oauth.vk.com/blank.html"
                   "&scope=wall,photos,groups,offline"
                   "&response_type=token"
                   "&v=5.130"));

    mVkAuthorizationView->show();
}

void MainWindow::setInterfaceListWidget()
{
    QListWidgetItem *waitingItem = new QListWidgetItem(ui->interfaceListWidget);
    waitingItem->setData(Qt::DisplayRole, "Ожидание");
    waitingItem->setData(Qt::UserRole, Waiting);
    waitingItem->setIcon(QIcon(":/style/image/hourglass.png"));

    QListWidgetItem *finishedItem = new QListWidgetItem(ui->interfaceListWidget);
    finishedItem->setData(Qt::DisplayRole, "Завершено");
    finishedItem->setData(Qt::UserRole, Finished);
    finishedItem->setIcon(QIcon(":/style/image/check.png"));

    QListWidgetItem *launchedItem = new QListWidgetItem(ui->interfaceListWidget);
    launchedItem->setData(Qt::DisplayRole, "В работе");
    launchedItem->setData(Qt::UserRole, Launched);
    launchedItem->setIcon(QIcon(":/style/image/shuttle.png"));

    QListWidgetItem *horizontalLineItem = new QListWidgetItem(ui->interfaceListWidget);
    horizontalLineItem->setData(Qt::UserRole, -1);
    horizontalLineItem->setData(Qt::UserRole + 1, true);
    horizontalLineItem->setFlags(Qt::NoItemFlags);

    QListWidgetItem *settingsItem = new QListWidgetItem(ui->interfaceListWidget);
    settingsItem->setData(Qt::DisplayRole, "Настройки");
    settingsItem->setData(Qt::UserRole, Settings);
    settingsItem->setIcon(QIcon(":/style/image/settings.png"));

    ui->interfaceListWidget->setItemDelegate(new InterfaceListDelegate(ui->interfaceListWidget));
}

QWidget *MainWindow::createNothingHereWidget()
{
    QWidget *nothingHereWidget = new QWidget(ui->tabFrame);
    nothingHereWidget->setStyleSheet("border-radius: 4px; "
                                      "background-color: white; "
                                      "color: #55677d;");
    QHBoxLayout *nothingHereWidgetLayout = new QHBoxLayout(nothingHereWidget);
    nothingHereWidgetLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *nothingHereWidgetLabel = new QLabel("Нажмите на меню слева для начала работы", nothingHereWidget);
    nothingHereWidgetLayout->addWidget(nothingHereWidgetLabel, 0, Qt::AlignHCenter);
    return nothingHereWidget;
}

QPixmap MainWindow::roundPhoto100(QPixmap photo)
{
    QSize photoSize(100, 100);
    QPixmap result(photoSize);
    result.fill(Qt::transparent);
    QPainter painter(&result);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path;
    path.addRoundedRect(0, 0, 100, 100, 50, 50);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, photo);
    return result;
}

void MainWindow::hideAllTabs()
{
    for (int i = 0; i < ui->tabFrame->layout()->count(); ++i)
    {
        QWidget *w = ui->tabFrame->layout()->itemAt(i)->widget();
        if(w != nullptr)
            w->hide();
    }
}

void MainWindow::setTabs()
{
    mNothingHereWidget = createNothingHereWidget();
    ui->tabFrame->layout()->addWidget(mNothingHereWidget);

    mWaitingWidget = new WaitingWidget(ui->tabFrame);
    ui->tabFrame->layout()->addWidget(mWaitingWidget);
    connect(mWaitingWidget, &WaitingWidget::showWidget,
            [&]()
    {
        hideAllTabs();
        mWaitingWidget->show();
    });
//    QWidget *finishedWidget = new QWidget(ui->tabFrame);

//    QWidget *launchedWidget = new QWidget(ui->tabFrame);

//    QWidget *settingsWidget = new QWidget(ui->tabFrame);
}

