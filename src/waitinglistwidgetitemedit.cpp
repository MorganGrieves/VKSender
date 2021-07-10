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
    //mDocAction = paperClipButtonMenu->addAction(QIcon(":/style/image/file.png"), "Документ");
    mAudioAction = paperClipButtonMenu->addAction(QIcon(":/style/image/music.png"), "Аудио");

    ui->paperClipButton->setMenu(paperClipButtonMenu);

    connect(ui->saveButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onSaveButtonReleased);
    connect(ui->cancelButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onCancelButtonReleased);
    connect(ui->deleteSelectedButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onDeleteSelectedButtonReleased);
    connect(ui->addGroupButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onAddGroupButtonReleased);
    connect(ui->backButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onBackButtonReleased);
    connect(ui->launchButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onLaunchButtonReleased);
    connect(ui->photoListWidget, &QListView::doubleClicked,
            this, &WaitingListWidgetItemEdit::onPhotoListWidgetDoubleClicked);
    connect(ui->videoListWidget, &QListView::doubleClicked,
            this, &WaitingListWidgetItemEdit::onVideoListWidgetDoubleClicked);
    connect(ui->audioListWidget, &QListView::doubleClicked,
            this, &WaitingListWidgetItemEdit::onAudioListWidgetDoubleClicked);
    connect(ui->docListWidget, &QListView::doubleClicked,
            this, &WaitingListWidgetItemEdit::onDocListWidgetDoubleClicked);

    connect(mPhotoAction, &QAction::triggered,
            [this](bool)
    {
        if (!canAddAttachment())
        {
            QMessageBox::warning(this, tr("VK Sender"), tr("Загрузить можно только 10 приложений!"));
            return;
        }

        QString fileName = QFileDialog::getOpenFileName(this, "Открыть", "",
                                                    tr("Image Files (*.png *.jpg *.jpeg *.gif)"));
        if (fileName.isEmpty()) return;

        if (QFileInfo(fileName).size() > 209715200)
        {
            QMessageBox::warning(this, tr("VK Sender"), tr("Файл не должен превышать 200 МБ!"));
            return;
        }

        if (photoIncludes(fileName))
        {
            QMessageBox::warning(this, tr("VK Sender"), tr("Данное изображение уже есть в списке!"));
            return;
        }

        QFile photoFile(fileName);
        if (photoFile.open(QIODevice::ReadOnly))
        {
            QListWidgetItem* listItem = new QListWidgetItem();
            listItem->setIcon(QPixmap(fileName));
            listItem->setToolTip(QFileInfo(fileName).fileName());
            listItem->setData(PathRole, fileName);
            listItem->setData(DataRole, photoFile.readAll());
            ui->photoListWidget->addItem(listItem);
            photoFile.close();
        }

        mSaveFlag = true;
    });

    connect(mAudioAction, &QAction::triggered,
            [this](bool)
    {
        if (!canAddAttachment())
        {
            QMessageBox::warning(this, tr("VK Sender"), tr("Загрузить можно только 10 приложений!"));
            return;
        }

        QString fileName = QFileDialog::getOpenFileName(this, "Открыть", "",
                                                    tr("Music Files (*.mp3)"));
        if (fileName.isEmpty()) return;

        if (QFileInfo(fileName).size() > 209715200)
        {
            QMessageBox::warning(this, tr("VK Sender"), tr("Файл не должен превышать 200 МБ!"));
            return;
        }

        if (audioIncludes(fileName))
        {
            QMessageBox::warning(this, tr("VK Sender"), tr("Данное аудио уже есть в списке"));
            return;
        }

        QFile audioFile(fileName);
        if (audioFile.open(QIODevice::ReadOnly))
        {
            QListWidgetItem* listItem = new QListWidgetItem();
            listItem->setIcon(QPixmap(":/style/image/doc_icons.png").copy(0, 90, 30, 30));
            listItem->setText(QFileInfo(fileName).fileName());
            listItem->setToolTip(QFileInfo(fileName).fileName());
            listItem->setData(PathRole, fileName);
            listItem->setData(DataRole, audioFile.readAll());
            ui->audioListWidget->addItem(listItem);
            audioFile.close();
        }

        mSaveFlag = true;
    });

    connect(mVideoAction, &QAction::triggered,
            [this](bool)
    {
        if (!canAddAttachment())
        {
            QMessageBox::warning(this, tr("VK Sender"), tr("Загрузить можно только 10 приложений!"));
            return;
        }

        QString fileName = QFileDialog::getOpenFileName(this, "Открыть", "",
                                                    tr("Video Files (*.avi *.mp4 *.3gp *.mpeg *.mov *.flv *.wmv)"));
        if (fileName.isEmpty()) return;

        if (QFileInfo(fileName).size() > 209715200)
        {
            QMessageBox::warning(this, tr("VK Sender"), tr("Файл не должен превышать 200 МБ!"));
            return;
        }

        if (videoIncludes(fileName))
        {
            QMessageBox::warning(this, tr("VK Sender"), tr("Данное видео уже есть в списке"));
            return;
        }

        QFile videoFile(fileName);
        if (videoFile.open(QIODevice::ReadOnly))
        {
            QListWidgetItem* listItem = new QListWidgetItem();
            listItem->setIcon(QPixmap(":/style/image/doc_icons.png").copy(0, 120, 30, 30));
            listItem->setText(QFileInfo(fileName).fileName());
            listItem->setToolTip(QFileInfo(fileName).fileName());
            listItem->setData(PathRole, fileName);
            listItem->setData(DataRole, videoFile.readAll());
            ui->videoListWidget->addItem(listItem);
            videoFile.close();
        }

        mSaveFlag = true;
    });

//    connect(mDocAction, &QAction::triggered,
//            [this](bool)
//    {
//        if (!canAddAttachment())
//        {
//            QMessageBox::warning(this, tr("VK Sender"), tr("Загрузить можно только 10 приложений!"));
//            return;
//        }

//        QString fileName = QFileDialog::getOpenFileName(this, "Открыть", "",
//                                                    tr("All files (*)"));
//        if (fileName.isEmpty()) return;

//        if (QFileInfo(fileName).size() > 209715200)
//        {
//            QMessageBox::warning(this, tr("VK Sender"), tr("Файл не должен превышать 200 МБ!"));
//            return;
//        }

//        if (videoIncludes(fileName))
//        {
//            QMessageBox::warning(this, tr("VK Sender"), tr("Данный файл уже есть в списке"));
//            return;
//        }

//        if (!acceptDocFile(fileName))
//        {
//            QMessageBox::warning(this, tr("VK Sender"), tr("Допустимы любые форматы за исключением mp3 и исполняемых файлов"));
//            return;
//        }

//        QFile docFile(fileName);
//        if (docFile.open(QIODevice::ReadOnly))
//        {
//            QListWidgetItem* listItem = new QListWidgetItem();
//            listItem->setIcon(QPixmap(":/style/image/doc_icons.png").copy(0, 0, 30, 30));
//            listItem->setText(QFileInfo(fileName).fileName());
//            listItem->setToolTip(QFileInfo(fileName).fileName());
//            listItem->setData(PathRole, fileName);
//            listItem->setData(DataRole, docFile.readAll());
//            ui->docListWidget->addItem(listItem);
//            docFile.close();
//        }

//        mSaveFlag = true;
//    });

}

WaitingListWidgetItemEdit::WaitingListWidgetItemEdit(const WaitingListWidgetItemEdit &item) :
    WaitingListWidgetItemEdit(nullptr)
{
    if (QStandardItemModel *itemModel = static_cast<QStandardItemModel *>(item.mGroupList->model()))
        for(int i = 0; i < item.mGroupList->model()->rowCount(); ++i)
            static_cast<QStandardItemModel *>(mGroupList->model())->appendRow(itemModel->item(i)->clone());    

    mFetcher = item.mFetcher;

    for (int i = 0; i < item.ui->photoListWidget->count(); ++i)
        ui->photoListWidget->addItem(item.ui->photoListWidget->item(i)->clone());

    for (int i = 0; i < item.ui->videoListWidget->count(); ++i)
        ui->videoListWidget->addItem(item.ui->videoListWidget->item(i)->clone());

    for (int i = 0; i < item.ui->audioListWidget->count(); ++i)
        ui->audioListWidget->addItem(item.ui->audioListWidget->item(i)->clone());

    for (int i = 0; i < item.ui->docListWidget->count(); ++i)
        ui->docListWidget->addItem(item.ui->docListWidget->item(i)->clone());

    ui->messageTextEdit->setMarkdown(item.ui->messageTextEdit->toMarkdown());
    ui->namePackLineEdit->setText(item.ui->namePackLineEdit->text());

    connect(mFetcher.get(), &Fetcher::userGroupsUpdate, this, &WaitingListWidgetItemEdit::onUserGroupsUpdate);
    connect(ui->messageTextEdit, &QTextEdit::textChanged, [this](){ mSaveFlag = true; });
    connect(ui->namePackLineEdit, &QLineEdit::textChanged, [this](const QString &){ mSaveFlag = true; });
    connect(mGroupList, &QListView::clicked, [this](const QModelIndex &)   { mSaveFlag = true; });
}

void WaitingListWidgetItemEdit::setFetcher(const std::shared_ptr<Fetcher> fetcher)
{
    mFetcher = fetcher;
    connect(mFetcher.get(), &Fetcher::userGroupsUpdate,
            this, &WaitingListWidgetItemEdit::onUserGroupsUpdate);

    for (const auto &group : fetcher->getUserGroups())
        addUserGroupListItem(group);

    connect(mFetcher.get(), &Fetcher::sortedGroupData,
            [this](QVector<QPair<Group, Qt::CheckState>> groups, QUuid id)
    {
        if (mId == id)
            for (auto &group : groups)
               addUserGroupListItem(group.first, group.second);
    });

    connect(mFetcher.get(), &Fetcher::updatedGroupData,
            [this](Group group, QUuid id)
    {
        if (mId == id)
            addUserGroupListItem(group);
    });
}

QString WaitingListWidgetItemEdit::getPackName() const
{
    return ui->namePackLineEdit->text();
}

void WaitingListWidgetItemEdit::setMessagePack(const MessagePack *message)
{
    ui->namePackLineEdit->setText((message->title == "Нет названия") ? "" : message->title);
    ui->messageTextEdit->setText(message->message);

    mFetcher->sortGroupsByCanPost(message->groups, mId);

    static_cast<QStandardItemModel *>(mGroupList->model())->clear();

    for (const auto &file : message->photoPaths)
    {
        QListWidgetItem* listItem = new QListWidgetItem();
        listItem->setIcon(QPixmap::fromImage(QImage(file.first)));
        listItem->setToolTip(QFileInfo(file.first).fileName());
        listItem->setData(PathRole, file.first);
        listItem->setData(DataRole, file.second);
        ui->photoListWidget->addItem(listItem);
    }

    for (const auto &file : message->videoPaths)
    {
        QListWidgetItem* listItem = new QListWidgetItem();
        listItem->setIcon(QPixmap(":/style/image/doc_icons.png").copy(0, 120, 30, 30));
        listItem->setText(QFileInfo(file.first).fileName());
        listItem->setToolTip(QFileInfo(file.first).fileName());
        listItem->setData(PathRole, file.first);
        listItem->setData(DataRole, file.second);
        ui->videoListWidget->addItem(listItem);
    }

    for (const auto &file : message->audioPaths)
    {
        QListWidgetItem* listItem = new QListWidgetItem();
        listItem->setIcon(QPixmap(":/style/image/doc_icons.png").copy(0, 90, 30, 30));
        listItem->setText(QFileInfo(file.first).fileName());
        listItem->setToolTip(QFileInfo(file.first).fileName());
        listItem->setData(PathRole, file.first);
        listItem->setData(DataRole, file.second);
        ui->audioListWidget->addItem(listItem);
    }

    for (const auto &file : message->docPaths)
    {
        QListWidgetItem* listItem = new QListWidgetItem();
        listItem->setIcon(QPixmap(":/style/image/doc_icons.png").copy(0, 0, 30, 30));
        listItem->setText(QFileInfo(file.first).fileName());
        listItem->setToolTip(QFileInfo(file.first).fileName());
        listItem->setData(PathRole, file.first);
        listItem->setData(DataRole, file.second);
        ui->docListWidget->addItem(listItem);
    }
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
    result.id = mId;
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
            group.type = itemModel->item(i)->data(GroupListDelegate::GROUP_TYPE).toString();

            result.groups.push_back(QPair<Group, Qt::CheckState>(group,
                                                                 itemModel->item(i)->checkState()));
        }

    result.message = ui->messageTextEdit->toPlainText();
    result.title = ui->namePackLineEdit->text().isEmpty() ? "Нет названия"
                                                          : ui->namePackLineEdit->text();

    for (int i = 0; i < ui->photoListWidget->count(); ++i)
        result.photoPaths.push_back(QPair(ui->photoListWidget->item(i)->data(PathRole).toString(),
                                          ui->photoListWidget->item(i)->data(DataRole).toByteArray()));

    for (int i = 0; i < ui->videoListWidget->count(); ++i)
        result.videoPaths.push_back(QPair(ui->videoListWidget->item(i)->data(PathRole).toString(),
                                          ui->videoListWidget->item(i)->data(DataRole).toByteArray()));

    for (int i = 0; i < ui->audioListWidget->count(); ++i)
        result.audioPaths.push_back(QPair(ui->audioListWidget->item(i)->data(PathRole).toString(),
                                          ui->audioListWidget->item(i)->data(DataRole).toByteArray()));


    for (int i = 0; i < ui->docListWidget->count(); ++i)
        result.docPaths.push_back(QPair(ui->docListWidget->item(i)->data(PathRole).toString(),
                                        ui->docListWidget->item(i)->data(DataRole).toByteArray()));

    return result;
}

void WaitingListWidgetItemEdit::setId(const QUuid &id)
{
    mId = id;
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
    emit backButtonReleased();
}

void WaitingListWidgetItemEdit::onUserGroupsUpdate()
{
    for (const auto &group : mFetcher->getUserGroups())
        addUserGroupListItem(group);
}

void WaitingListWidgetItemEdit::onPhotoListWidgetDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (QListWidget *photoListWidget = dynamic_cast <QListWidget *>(sender()))
        if (QListWidgetItem *photoItem = photoListWidget->takeItem(index.row()))
            delete photoItem;
}

void WaitingListWidgetItemEdit::onVideoListWidgetDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (QListWidget *videoListWidget = dynamic_cast <QListWidget *>(sender()))
        if (QListWidgetItem *videoItem = videoListWidget->takeItem(index.row()))
            delete videoItem;
}

void WaitingListWidgetItemEdit::onAudioListWidgetDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (QListWidget *audioListWidget = dynamic_cast <QListWidget *>(sender()))
        if (QListWidgetItem *audioItem = audioListWidget->takeItem(index.row()))
            delete audioItem;
}

void WaitingListWidgetItemEdit::onDocListWidgetDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (QListWidget *docListWidget = dynamic_cast <QListWidget *>(sender()))
        if (QListWidgetItem *docItem = docListWidget->takeItem(index.row()))
            delete docItem;
}

void WaitingListWidgetItemEdit::onDeleteSelectedButtonReleased()
{
    QList<QListWidgetItem *> selectedItems = ui->photoListWidget->selectedItems();
    for (QListWidgetItem *item : selectedItems)
        delete item;

    selectedItems = ui->videoListWidget->selectedItems();
    for (QListWidgetItem *item : selectedItems)
        delete item;

    selectedItems = ui->audioListWidget->selectedItems();
    for (QListWidgetItem *item : selectedItems)
        delete item;

    selectedItems = ui->docListWidget->selectedItems();
    for (QListWidgetItem *item : selectedItems)
        delete item;
}

void WaitingListWidgetItemEdit::onAddGroupButtonReleased()
{
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


    mFetcher->getGroupById(filterGroupLineEdit(ui->groupLinkEdit->text()), mId);
}

void WaitingListWidgetItemEdit::onBackButtonReleased()
{
    if (mSaveFlag)
    {
        QMessageBox::StandardButton userAction =
                QMessageBox::question(this, "VKSender", "Сохранить все изменения?",
                                      QMessageBox::Yes|QMessageBox::No);

        if (userAction == QMessageBox::Close)
            return;

        if (userAction == QMessageBox::Yes)
            emit saveButtonReleased();
    }

    emit backButtonReleased();
}

void WaitingListWidgetItemEdit::onLaunchButtonReleased()
{
    if (ui->messageTextEdit->toPlainText().isEmpty() && isAttchmentsEmpty())
    {
        QMessageBox::warning(this, tr("VK Sender"), tr("Напишите сообщение или добавьте приложение."));
        return;
    }
    bool checked = false;
    bool pageInclued = false;
    if (QStandardItemModel *itemModel = static_cast<QStandardItemModel *>(mGroupList->model()))
    {
        for(int i = 0; i < itemModel->rowCount(); ++i)
        {
            if (itemModel->item(i)->data(Qt::CheckStateRole) == Qt::Checked)
                checked = true;
            if (itemModel->item(i)->data(GroupListDelegate::GROUP_TYPE) == "page")
                pageInclued = true;
        }

        if (!checked)
        {
            QMessageBox::warning(this, tr("VK Sender"), tr("Не выбрана ни одна группа для рассылки"));
            return;
        }
    }

    emit saveButtonReleased();
    emit launchButtonReleased();
}

void WaitingListWidgetItemEdit::addUserGroupListItem(const Group &group, Qt::CheckState state)
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
    item->setData(group.type, GroupListDelegate::GROUP_TYPE);
    item->setCheckState(state);

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
    ui->groupListView->layout()->addWidget(mGroupList);
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

int WaitingListWidgetItemEdit::countAllAttachments() const
{
    return ui->photoListWidget->count() +
            ui->videoListWidget->count() +
            ui->audioListWidget->count() +
            ui->docListWidget->count();
}

bool WaitingListWidgetItemEdit::canAddAttachment() const
{
    if (countAllAttachments() > 10)
        return false;

    return true;
}

bool WaitingListWidgetItemEdit::photoIncludes(const QString &fileName)
{
    for (int i = 0; i < ui->photoListWidget->count(); ++i)
    {
        if (ui->photoListWidget->item(i)->data(PathRole).toString() == fileName)
            return true;
    }

    return false;
}

bool WaitingListWidgetItemEdit::audioIncludes(const QString &fileName)
{
    for (int i = 0; i < ui->audioListWidget->count(); ++i)
        if (ui->audioListWidget->item(i)->data(PathRole).toString() == fileName)
            return true;

    return false;
}

bool WaitingListWidgetItemEdit::videoIncludes(const QString &fileName)
{
    for (int i = 0; i < ui->videoListWidget->count(); ++i)
        if (ui->videoListWidget->item(i)->data(PathRole).toString() == fileName)
            return true;

    return false;
}

bool WaitingListWidgetItemEdit::docIncluded(const QString &fileName)
{
    for (int i = 0; i < ui->docListWidget->count(); ++i)
        if (ui->docListWidget->item(i)->data(PathRole).toString() == fileName)
            return true;

    return false;
}

bool WaitingListWidgetItemEdit::acceptDocFile(const QString &fileName) const
{
    QFileInfo docFileInfo(fileName);
    if (docFileInfo.suffix() == "exe" || docFileInfo.suffix() == "mp3")
        return false;

    return true;
}

bool WaitingListWidgetItemEdit::isAttchmentsEmpty() const
{
    if (ui->photoListWidget->count() ||
            ui->docListWidget->count() ||
            ui->audioListWidget->count() ||
            ui->videoListWidget->count())
        return false;

    return true;
}

WaitingListWidgetItemEdit::~WaitingListWidgetItemEdit()
{
    delete ui;
}
