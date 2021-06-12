#ifndef GROUPLISTVIEW_H
#define GROUPLISTVIEW_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QListView>
#include <QStandardItemModel>
#include <QApplication>
#include <QMouseEvent>

class GroupListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    enum DataType
    {
        GROUP_NAME = Qt::UserRole,
        GROUP_ICON,
        GROUP_LINK,
        GROUP_SCREENNAME,
        GROUP_ID
    };
    explicit GroupListDelegate(QObject *parent = nullptr);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &index) const;

};

class GroupListView : public QListView
{
    Q_OBJECT

public:
    explicit GroupListView(QWidget *parent = nullptr);

protected:
    void mouseMoveEvent(QMouseEvent *e) override;
    
private slots:
    void onItemClicked(const QModelIndex &index);

};

#endif // GROUPLISTVIEW_H
