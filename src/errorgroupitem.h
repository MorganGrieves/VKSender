#ifndef ERRORGROUPITEM_H
#define ERRORGROUPITEM_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>

#include "types.h"

namespace Ui {
class ErrorGroupItem;
}

class ErrorGroupItem : public QWidget
{
    Q_OBJECT

public:
    explicit ErrorGroupItem(QWidget *parent = nullptr);
    ~ErrorGroupItem();
    void setGroup(Group group);
    QPixmap roundPhoto35(QPixmap photo) const;

private:
    Ui::ErrorGroupItem *ui;

    Group mGroup;
};

#endif // ERRORGROUPITEM_H
