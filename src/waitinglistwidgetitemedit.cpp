#include "waitinglistwidgetitemedit.h"
#include "ui_waitinglistwidgetitemedit.h"

#include <QMessageBox>

WaitingListWidgetItemEdit::WaitingListWidgetItemEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaitingListWidgetItemEdit)
{
    ui->setupUi(this);

    createGroupListView();

    QMenu *paperClipButtonMenu = new QMenu(ui->paperClipButton);
    paperClipButtonMenu->setCursor(Qt::PointingHandCursor);
    mPhotoAction = paperClipButtonMenu->addAction(QIcon(":/style/image/camera.png"), "Фотография");
    mVideoAction = paperClipButtonMenu->addAction(QIcon(":/style/image/play-button.png"), "Видеозапись");
    mFileAction = paperClipButtonMenu->addAction(QIcon(":/style/image/file.png"), "Документ");

    ui->paperClipButton->setMenu(paperClipButtonMenu);

    connect(ui->saveButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onSaveButtonReleased);
    connect(ui->cancelButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onCancelButtonReleased);
    connect(ui->deleteSelectedButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onDeleteSelectedButtonReleased);
    connect(ui->photoListWidget, &QListView::doubleClicked,
            this, &WaitingListWidgetItemEdit::onPhotosListViewDoubleClicked);
    connect(ui->addGroupButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onAddGroupButtonReleased);
    connect(ui->backButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onBackButtonReleased);
    connect(ui->launchButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onLaunchButtonReleased);

    connect(mPhotoAction, &QAction::triggered,
            [this](bool)
    {
        if (ui->photoListWidget->count() >= 6)
        {
            QMessageBox::warning(this, tr("VK Sender"), tr("Загрузить можно только 6 изображений"));
            return;
        }

        QString name = QFileDialog::getOpenFileName(0, "Открыть", "",
                                                    tr("Image Files (*.png *.jpg *.jpeg *.gif)"));
        if (mPhotoPaths.find(name) != mPhotoPaths.end())
        {
            QMessageBox::warning(this, tr("VK Sender"), tr("Данное изображение уже есть в списке"));
            return;
        }

        QListWidgetItem* listItem = new QListWidgetItem();
        listItem->setIcon(QPixmap(name));
        ui->photoListWidget->addItem(listItem);
        mPhotoPaths[name] = listItem;
        mSaveFlag = true;
    });
}

WaitingListWidgetItemEdit::WaitingListWidgetItemEdit(const WaitingListWidgetItemEdit &item) :
    WaitingListWidgetItemEdit(nullptr)
{
    if (QStandardItemModel *itemModel = static_cast<QStandardItemModel *>(item.mGroupList->model()))
    {
        for(int i = 0; i < item.mGroupList->model()->rowCount(); ++i)
            static_cast<QStandardItemModel *>(mGroupList->model())->appendRow(itemModel->item(i)->clone());
    }

    mFetcher = item.mFetcher;
    connect(mFetcher.get(), &Fetcher::userGroupsUpdate,
            this, &WaitingListWidgetItemEdit::onUserGroupsUpdate);

    for (const auto & photoPath : item.mPhotoPaths)
    {
        QListWidgetItem * clone = photoPath.second->clone();
        mPhotoPaths[photoPath.first] = clone;
        ui->photoListWidget->addItem(clone);
    }

    ui->messageTextEdit->setMarkdown(item.ui->messageTextEdit->toMarkdown());

    connect(ui->messageTextEdit, &QTextEdit::textChanged,
            [&]()
    {
        mSaveFlag = true;
    });

    ui->namePackLineEdit->setText(item.ui->namePackLineEdit->text());

    connect(ui->namePackLineEdit, &QLineEdit::textChanged,
            [&](const QString &)
    {
        mSaveFlag = true;
    });

    connect(mGroupList, &QListView::clicked,
            [&](const QModelIndex &)
    {
        mSaveFlag = true;
    });

}

void WaitingListWidgetItemEdit::setFetcher(const std::shared_ptr<Fetcher> fetcher)
{
    mFetcher = fetcher;
    connect(mFetcher.get(), &Fetcher::userGroupsUpdate,
            this, &WaitingListWidgetItemEdit::onUserGroupsUpdate);
    for (const auto &group : fetcher->getUserGroups())
        addUserGroupListItem(group);
}

QString WaitingListWidgetItemEdit::getPackName() const
{
    return ui->namePackLineEdit->text();
}

int WaitingListWidgetItemEdit::getCheckedGroupsNumber()
{
    int checkedGroups = 0;

    if (QStandardItemModel *itemModel = static_cast<QStandardItemModel *>(mGroupList->model()))
        for(int i = 0; i < itemModel->rowCount(); ++i)
           if (itemModel->item(i)->data(Qt::CheckStateRole) == Qt::Checked)
               checkedGroups++;

    return checkedGroups;
}

MessagePack WaitingListWidgetItemEdit::getMessageInfo() const
{
    MessagePack result;
    result.id = QUuid::createUuid();
    if (QStandardItemModel *itemModel = static_cast<QStandardItemModel *>(mGroupList->model()))
        for(int i = 0; i < itemModel->rowCount(); ++i)
        {
            Group group;

            group.vkid = itemModel->item(i)->data(GroupListDelegate::GROUP_ID).toString();
            group.name = itemModel->item(i)->data(GroupListDelegate::GROUP_NAME).toString();
            group.screenName = itemModel->item(i)->data(GroupListDelegate::GROUP_SCREENNAME).toString();
            group.photo50Link = itemModel->item(i)->data(GroupListDelegate::GROUP_PHOTO50LINK).toString();
            group.photo50 = itemModel->item(i)->data(GroupListDelegate::GROUP_ICON).value<QPixmap>();
            group.canPost = itemModel->item(i)->data(GroupListDelegate::GROUP_CANPOST).toBool();

            result.groups.push_back(QPair<Group, Qt::CheckState>(group,
                                                                 itemModel->item(i)->checkState()));
        }

    result.message = ui->messageTextEdit->toPlainText();
    result.title = ui->namePackLineEdit->text();
    for (const auto &[Path, _] : mPhotoPaths)
        result.photoPaths.push_back(Path);

    return result;
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

void WaitingListWidgetItemEdit::onSaveButtonReleased()
{
    emit saveButtonReleased();

    emit backButtonReleased();
}

void WaitingListWidgetItemEdit::onCancelButtonReleased()
{
    emit cancelButtonReleased();
}

void WaitingListWidgetItemEdit::onUserGroupsUpdate()
{
    for (const auto &group : mFetcher->getUserGroups())
        addUserGroupListItem(group);
}

void WaitingListWidgetItemEdit::onPhotosListViewDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (QListWidget *photosListWidget = dynamic_cast <QListWidget *>(sender()))
        if (QListWidgetItem *photoItem = photosListWidget->takeItem(index.row()))
            for (auto it = mPhotoPaths.begin(); it != mPhotoPaths.end(); )
            {
                if (it->second == photoItem)
                {
                    delete photoItem;
                    it = mPhotoPaths.erase(it);
                }
                else
                    ++it;
            }
}

void WaitingListWidgetItemEdit::onDeleteSelectedButtonReleased()
{
    QList<QListWidgetItem *> selectedItems = ui->photoListWidget->selectedItems();
    for (QListWidgetItem *item : selectedItems)
        for (auto it = mPhotoPaths.begin(); it != mPhotoPaths.end(); )
        {
            if (it->second == item)
            {
                delete item;
                it = mPhotoPaths.erase(it);
            }
            else
                ++it;
        }
}

void WaitingListWidgetItemEdit::onAddGroupButtonReleased()
{
    connect(mFetcher.get(), &Fetcher::onGroupUpdated,
            [&](Group group)
    {
        if (group.canPost)
            addUserGroupListItem(group);
    });

    if (QStandardItemModel *itemModel = static_cast<QStandardItemModel *>(mGroupList->model()))
    {
        for(int i = 0; i < itemModel->rowCount(); ++i)
            if (itemModel->item(i)->data(GroupListDelegate::GROUP_SCREENNAME).toString()
                    == ui->groupLinkEdit->text()
                    || itemModel->item(i)->data(GroupListDelegate::GROUP_ID).toString()
                    == filterGroupLineEdit(ui->groupLinkEdit->text()))
            {
                QMessageBox::warning(this, tr("VK Sender"), tr("Данная ссылка уже есть в списке!"));
                ui->groupLinkEdit->clear();
                return;
            }
    }

    mFetcher->getGroupInfoById(filterGroupLineEdit(ui->groupLinkEdit->text()));
}

void WaitingListWidgetItemEdit::onBackButtonReleased()
{
    if (mSaveFlag)
        if (QMessageBox::question(this, "VKSender", "Сохранить все изменения?",
                                  QMessageBox::Yes|QMessageBox::No)
                == QMessageBox::Yes)
        {
            emit saveButtonReleased();
        }

    emit backButtonReleased();
}

void WaitingListWidgetItemEdit::onLaunchButtonReleased()
{
    emit saveButtonReleased();
    emit launchButtonReleased();
}

void WaitingListWidgetItemEdit::addUserGroupListItem(const Group &group)
{
    QStandardItemModel *model = static_cast<QStandardItemModel *>(mGroupList->model());

    QStandardItem *item = new QStandardItem();
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    item->setData(group.name, GroupListDelegate::GROUP_NAME);
    item->setData(roundPhoto35(group.photo50), GroupListDelegate::GROUP_ICON);
    item->setData(group.screenName, GroupListDelegate::GROUP_SCREENNAME);
    item->setData(group.vkid, GroupListDelegate::GROUP_ID);
    item->setData(group.canPost, GroupListDelegate::GROUP_CANPOST);
    item->setData(group.photo50Link, GroupListDelegate::GROUP_PHOTO50LINK);

    item->setSizeHint(QSize(0, 60));
    model->appendRow(item);
    mSaveFlag = true;
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

QString WaitingListWidgetItemEdit::filterGroupLineEdit(const QString &text)
{
    QRegExp reg("(\\b(public|club)[\\d]+$)");
    if (reg.indexIn(text) != -1)
    {
        QRegExp reg1("[\\d]+");
        reg1.indexIn(reg.cap(0));
        return reg1.cap(0);
    }

    return text;
}

WaitingListWidgetItemEdit::~WaitingListWidgetItemEdit()
{
    delete ui;
}
