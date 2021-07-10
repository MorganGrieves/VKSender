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

    mVkAuthorizationDialog = new VkAuthorizationDialog(this);

    mRepository = std::make_shared<Repository>();
    mFetcher = std::make_shared<Fetcher>();

    setToken();
    setInterfaceListWidget();
    setTabs();
    hideAllTabs();
    setFetchers();
    setRepositories();
    setVersion();
    setLastEntrance();

    mNothingHereWidget->show();

    connect(ui->interfaceListWidget, &QListWidget::itemClicked,
            this, &MainWindow::onInterfaceListWidgetItemClicked);

    connect(mWaitingWidget, &WaitingWidget::waitingListWidgetItemReleased,
            [this](WaitingListWidgetItemEdit *item)
    {
        hideAllTabs();
        mEditFormShowed = true;
        ui->tabFrame->layout()->addWidget(item);
        ui->interfaceListWidget->clearSelection();
    });

    connect(mFinishedWidget, &FinishedWidget::finishedListWidgetItemReleased,
            [this](FinishedListWidgetItemEdit *item)
    {
        hideAllTabs();
        int index = ui->tabFrame->layout()->indexOf(item);
        if(index == -1)
            ui->tabFrame->layout()->addWidget(item);
        else
            ui->tabFrame->layout()->itemAt(index)->widget()->show();
        ui->interfaceListWidget->clearSelection();
    });

    connect(mFetcher.get(), &Fetcher::userPhoto100Update,
            this, &MainWindow::onProfilePictureUpdated);

    connect(mFetcher.get(), &Fetcher::userNameUpdate,
            this, &MainWindow::onUserNameUpdated);

    connect(ui->changeAccountButton, &QPushButton::released,
            this, &MainWindow::onChangeAccountButtonReleased);

    connect(mVkAuthorizationDialog, &VkAuthorizationDialog::accessTokenReceived,
            [this](QString *token)
    {
        mFetcher->setAccessToken(*token);
        mFetcher->onUserDataUpdate();
    });
}

MainWindow::~MainWindow()
{
    delete ui;

    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    settings.setValue("Mainwindow/lastEntranceSecs", QString::number(QDateTime::currentDateTime().toSecsSinceEpoch()));
    settings.setValue("Mainwindow/version.major", mVersion.major);
    settings.setValue("Mainwindow/version.minor", mVersion.minor);
    settings.setValue("Mainwindow/version.patch", mVersion.patch);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (!(mGreetingWidget == nullptr))
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

void MainWindow::onInterfaceListWidgetItemClicked(QListWidgetItem *item)
{
    if (mEditFormShowed)
    {
        ui->interfaceListWidget->clearSelection();
        QMessageBox::warning(this, tr("VK Sender"), tr("Сохраните или отмените изменения в редакторе!"));
        return;
    }
    switch (item->data(Qt::UserRole).toInt())
    {
    case Waiting:
        hideAllTabs();
        mWaitingWidget->show();
        break;
    case Finished:
        hideAllTabs();
        mFinishedWidget->show();
        break;
    case Launched:
        hideAllTabs();
        mLaunchedWidget->show();
        break;
    case Settings:
        //ui->nothingHereFrame->setHidden(true);
        break;
    }
}

void MainWindow::onChangeAccountButtonReleased()
{
    mVkAuthorizationDialog->exec();
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

    mFinishedWidget = new FinishedWidget(ui->tabFrame);
    ui->tabFrame->layout()->addWidget(mFinishedWidget);
    connect(mFinishedWidget, &FinishedWidget::showWidget,
            [this]()
    {
        hideAllTabs();
        ui->interfaceListWidget->selectionModel()->select(
                    ui->interfaceListWidget->model()->index(1, 0),
                    QItemSelectionModel::Clear | QItemSelectionModel::Select);

        mFinishedWidget->show();
    });
    connect(mFinishedWidget, &FinishedWidget::abortionFinished,
            [this](MessagePack message)
    {
        mWaitingWidget->addListItem(&message);
        hideAllTabs();
        ui->interfaceListWidget->selectionModel()->select(
                    ui->interfaceListWidget->model()->index(0, 0),
                    QItemSelectionModel::Clear | QItemSelectionModel::Select);

        mWaitingWidget->show();
    });

    mLaunchedWidget = new LaunchedWidget(ui->tabFrame);
    ui->tabFrame->layout()->addWidget(mLaunchedWidget);
    connect(mLaunchedWidget, &LaunchedWidget::sendingFinished,
            [this](SendingResult result)
    {
        hideAllTabs();
        ui->interfaceListWidget->clearSelection();
        ui->interfaceListWidget->selectionModel()->select(
                    ui->interfaceListWidget->model()->index(1, 0),
                    QItemSelectionModel::Clear | QItemSelectionModel::Select);
        mFinishedWidget->addFinishedItem(result);
        mFinishedWidget->show();
    });

    mWaitingWidget = new WaitingWidget(ui->tabFrame);
    ui->tabFrame->layout()->addWidget(mWaitingWidget);

    connect(mWaitingWidget, &WaitingWidget::showWidget,
            [this]()
    {
        hideAllTabs();
        mEditFormShowed = false;
        ui->interfaceListWidget->selectionModel()->select(
                    ui->interfaceListWidget->model()->index(0, 0),
                    QItemSelectionModel::Clear | QItemSelectionModel::Select);
        mWaitingWidget->show();
    });
    connect(mWaitingWidget, &WaitingWidget::launchSending,
            [this](MessagePack message)
    {
        hideAllTabs();
        mEditFormShowed = false;
        ui->interfaceListWidget->selectionModel()->select(
                    ui->interfaceListWidget->model()->index(2, 0),
                    QItemSelectionModel::Clear | QItemSelectionModel::Select);
        mLaunchedWidget->show();
        mLaunchedWidget->addLaunchedItem(message);
    });
    connect(mWaitingWidget, &WaitingWidget::loadListsButtonRelease,
            [this]()
    {
        if (mFinishedWidget->listSize() + mLaunchedWidget->listSize() > 0)
        {
            QMessageBox::StandardButton userAction =
                    QMessageBox::question(this, "VKSender", "Выполняется рассылка. Все равно загрузить?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (userAction == QMessageBox::Yes)
                mWaitingWidget->loadLists();
            return;
        }
        mWaitingWidget->loadLists();
    });
    connect(mWaitingWidget, &WaitingWidget::saveListsButtonRelease,
            [this]()
    {
        if (mFinishedWidget->listSize() + mLaunchedWidget->listSize() > 0)
        {
            QMessageBox::StandardButton userAction =
                    QMessageBox::question(this, "VKSender", "Выполняется рассылка. Все равно сохранить?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (userAction == QMessageBox::Yes)
                mWaitingWidget->saveLists();
            return;
        }
        mWaitingWidget->saveLists();
    });
    //Здесь должен быть Settings widget
}

void MainWindow::setToken()
{
    if (hasToken())
    {
        mGreetingWidget = std::make_shared<GreetingWidget>(this);
        mGreetingWidget->resize(rect().size());
        mGreetingWidget->show();
        connect(mGreetingWidget.get(), &GreetingWidget::accessTokenReceived,
                [this](QString *token)
        {
            mGreetingWidget->close();
            mFetcher->setAccessToken(*token);
        });
    }
}

void MainWindow::setLastEntrance()
{
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
        ui->lastEntranceLabel->setText(labelText);
    }
}

void MainWindow::setFetchers()
{
    mWaitingWidget->setFetcher(mFetcher);
    mLaunchedWidget->setFetcher(mFetcher);
    mFinishedWidget->setFetcher(mFetcher);
}

void MainWindow::setRepositories()
{
    mFetcher->setRepository(mRepository);
    mWaitingWidget->setRepository(mRepository);
}

void MainWindow::setVersion()
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    mVersion.major = settings.value("Mainwindow/version.major").toInt();
    mVersion.minor = settings.value("Mainwindow/version.minor").toInt();
    mVersion.patch = settings.value("Mainwindow/version.patch").toInt();
}

bool MainWindow::hasToken()
{
    return mFetcher->tokenIsEmpty();
}
