#include "noerrorgroupitem.h"
#include "ui_noerrorgroupitem.h"

NoErrorGroupItem::NoErrorGroupItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NoErrorGroupItem)
{
    ui->setupUi(this);

    connect(ui->abortButton, &QPushButton::released,
            this, &NoErrorGroupItem::onAbortButtonReleased);
}

void NoErrorGroupItem::setFetcher(const std::shared_ptr<Fetcher> fetcher)
{
    mFetcher = fetcher;
}

void NoErrorGroupItem::setGroup(Group group)
{
    mGroup = group;
    ui->groupNameLabel->setText(group.name);
    ui->groupIconLabel->setPixmap(roundPhoto35(group.photo50));
}

void NoErrorGroupItem::setPostNumber(PostNumber number)
{
    mPostNumber = number;
}

QPixmap NoErrorGroupItem::roundPhoto35(QPixmap photo) const
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

void NoErrorGroupItem::onAbortButtonReleased()
{
    mFetcher->onPostDelete(mPostNumber, mGroup.vkid);
    emit abortButtonReleased(mGroup);
    this->deleteLater();
}

NoErrorGroupItem::~NoErrorGroupItem()
{
    delete ui;
}
