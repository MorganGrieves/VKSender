#ifndef NOERRORGROUPITEM_H
#define NOERRORGROUPITEM_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>

#include "types.h"
#include "fetcher.h"

namespace Ui {
class NoErrorGroupItem;
}

class NoErrorGroupItem : public QWidget
{
    Q_OBJECT

signals:
    void abortButtonReleased(Group group);

public:
    explicit NoErrorGroupItem(QWidget *parent = nullptr);
    void setFetcher(const std::shared_ptr<Fetcher> fetcher);
    void setGroup(Group group);
    void setPostNumber(PostNumber number);
    QPixmap roundPhoto35(QPixmap photo) const;
    ~NoErrorGroupItem();

private slots:
    void onAbortButtonReleased();

private:
    Ui::NoErrorGroupItem *ui;

    Group mGroup;
    PostNumber mPostNumber = 0;
    std::shared_ptr<Fetcher> mFetcher = nullptr;
};

#endif // NOERRORGROUPITEM_H
