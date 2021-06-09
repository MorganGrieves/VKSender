#include "waitinglistwidgetitemedit.h"
#include "ui_waitinglistwidgetitemedit.h"

WaitingListWidgetItemEdit::WaitingListWidgetItemEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaitingListWidgetItemEdit)
{
    ui->setupUi(this);

    QMenu *paperClipButtonMenu = new QMenu(ui->paperClipButton);
    paperClipButtonMenu->addAction(QIcon(":/style/image/camera.png"), "Фотография");
    paperClipButtonMenu->addAction(QIcon(":/style/image/play-button.png"), "Видеозапись");
    paperClipButtonMenu->addAction(QIcon(":/style/image/file.png"), "Документ");

    ui->paperClipButton->setMenu(paperClipButtonMenu);

//    connect(ui->userGroupListWidget, &QListWidget::itemClicked,
//            this, &WaitingListWidgetItemEdit::onGroupListWidgetItemClicked);
//    connect(ui->groupListWidget, &QListWidget::itemClicked,
//            this, &WaitingListWidgetItemEdit::onGroupListWidgetItemClicked);

    connect(ui->backButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onBackButtonReleased);
    connect(ui->saveButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onSaveButtonReleased);
    connect(ui->cancelButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onCancelButtonReleased);

    //ui->tabWidget->tabBar()->setCursor(Qt::PointingHandCursor);
}

void WaitingListWidgetItemEdit::setFetcher(const std::shared_ptr<Fetcher> fetcher)
{
    mFetcher = fetcher;
    for (const auto &group : fetcher->getUserGroups())
        addUserGroupListItem(group);
}

void WaitingListWidgetItemEdit::onGroupListWidgetItemClicked(QListWidgetItem *item)
{
    if(item == nullptr)
        return;

    if(item->data(Qt::CheckStateRole) != Qt::Checked)
        item->setData(Qt::CheckStateRole, Qt::Checked);
    else
        item->setData(Qt::CheckStateRole, Qt::Unchecked);
}

void WaitingListWidgetItemEdit::onBackButtonReleased()
{
    emit backButtonReleased();
}

void WaitingListWidgetItemEdit::onSaveButtonReleased()
{
    emit saveButtonReleased();
}

void WaitingListWidgetItemEdit::onCancelButtonReleased()
{
    emit cancelButtonReleased();
}

void WaitingListWidgetItemEdit::addUserGroupListItem(const Group &group)
{
    QStandardItemModel *model = static_cast<QStandardItemModel *>(mGroupList->model());

    QStandardItem *item = new QStandardItem();
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    item->setData(group.name, GroupListDelegate::GROUP_NAME);
    item->setData(roundPhoto35(group.photo50), GroupListDelegate::GROUP_ICON);
    item->setData("https://vk.com/" + group.screenName, GroupListDelegate::GROUP_LINK);
    item->setSizeHint(QSize(0, 60));
    model->appendRow(item);

}

QPixmap WaitingListWidgetItemEdit::roundPhoto35(QPixmap photo) const
{
    photo = photo.scaled(35, 35, Qt::IgnoreAspectRatio,Qt::SmoothTransformation);

    QPixmap result(photo.size());
    result.fill(Qt::transparent);

    QPainter painter(&result);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path;

    path.addRoundedRect(0, 0, 35, 35, 35 / 2, 35 / 2);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, photo);
    return result;
}

void WaitingListWidgetItemEdit::createGroupListView()
{
    mGroupList = new GroupListView(this);
    ui->selectedGroupsWidget->layout()->addWidget(mGroupList);
}

WaitingListWidgetItemEdit::~WaitingListWidgetItemEdit()
{
    delete ui;
}
