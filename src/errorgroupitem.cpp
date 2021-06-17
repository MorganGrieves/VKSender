#include "errorgroupitem.h"
#include "ui_errorgroupitem.h"

ErrorGroupItem::ErrorGroupItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ErrorGroupItem)
{
    ui->setupUi(this);
}

void ErrorGroupItem::setGroup(Group group)
{
    mGroup = group;
    ui->groupIconLabel->setPixmap(roundPhoto35(group.photo50));
    ui->groupNameLabel->setText(group.name);
}

QPixmap ErrorGroupItem::roundPhoto35(QPixmap photo) const
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

ErrorGroupItem::~ErrorGroupItem()
{
    delete ui;
}
