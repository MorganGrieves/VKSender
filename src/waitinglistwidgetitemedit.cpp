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
    qDebug() << this;

    addUserGroupListWidgetItem();
    addUserGroupListWidgetItem();
    addUserGroupListWidgetItem();
    addUserGroupListWidgetItem();
    addUserGroupListWidgetItem();

    connect(ui->userGroupListWidget, &QListWidget::itemClicked,
            this, &WaitingListWidgetItemEdit::onGroupListWidgetItemClicked);
    connect(ui->groupListWidget, &QListWidget::itemClicked,
            this, &WaitingListWidgetItemEdit::onGroupListWidgetItemClicked);

    connect(ui->backButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onBackButtonReleased);
    connect(ui->saveButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onSaveButtonReleased);
    connect(ui->cancelButton, &QPushButton::released,
            this, &WaitingListWidgetItemEdit::onCancelButtonReleased);

    ui->tabWidget->tabBar()->setCursor(Qt::PointingHandCursor);
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

void WaitingListWidgetItemEdit::addUserGroupListWidgetItem()
{
    QListWidgetItem *item = new QListWidgetItem(ui->userGroupListWidget);
    item->setText("Супер офигенное название для группы, блеяд");
    item->setIcon(roundPhoto75(QPixmap(":/style/image/photo.jpg")));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Unchecked);
    item->setFont(QFont("Roboto", 10));

    ui->userGroupListWidget->addItem(item);
}

QPixmap WaitingListWidgetItemEdit::roundPhoto75(QPixmap photo)
{
    QPixmap result(photo.size());
    result.fill(Qt::transparent);

    QPainter painter(&result);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path;
    path.addRoundedRect(0, 0, 75, 75, 75 / 2, 75 / 2);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, photo);
    return result;
}

WaitingListWidgetItemEdit::~WaitingListWidgetItemEdit()
{
    delete ui;
}
