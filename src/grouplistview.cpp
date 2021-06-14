#include "grouplistview.h"
#include <QDebug>
#include <QToolTip>
#include <QClipboard>

GroupListDelegate::GroupListDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void GroupListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString groupName = index.data(GROUP_NAME).toString(),
            fontFamily = "Roboto";
    QPixmap groupIcon = index.data(GROUP_ICON).value<QPixmap>();

    bool checked = index.data(Qt::CheckStateRole) == Qt::Checked,
            hovered = option.state & QStyle::State_MouseOver;

    QFont fntHeader(fontFamily, 9);
    QString colorGroupName = "black";

    int xContent = option.rect.x() + 10,
            yContent = option.rect.width() - 20,
            yMiddle = option.rect.center().y() - 5;

    painter->save();
    painter->setClipping(true);
    painter->setClipRect(option.rect);
    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setPen(QPen("#E2E4E5"));
    int lineBottom = option.rect.y() + option.rect.height() - 1;
    painter->drawLine(xContent, lineBottom, yContent, lineBottom);

    QPointF checkPoint (xContent + 25, yMiddle);
    if (checked)
        painter->drawPixmap(checkPoint, QPixmap(":/style/image/checkbox_checked.svg"));
    else
        painter->drawPixmap(checkPoint, QPixmap(":/style/image/checkbox_unchecked.svg"));

    QPointF groupIconPoint (checkPoint.x() + 25, yMiddle - 10);
    painter->drawPixmap(groupIconPoint, groupIcon);

    painter->setFont(fntHeader);
    int widthGroupName = painter->fontMetrics().horizontalAdvance(groupName);
    painter->setPen(QPen(colorGroupName));
    painter->drawText(QRect(groupIconPoint.x() + 45, yMiddle, widthGroupName, 32), Qt::TextSingleLine, groupName);

    QPixmap closeButton = QPixmap(":/style/image/close.png").scaled(15, 15, Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    painter->drawPixmap(yContent - 20, yMiddle + 5, closeButton);

    QPixmap copyLinkButton = QPixmap(":/style/image/copylink.png").scaled(15, 15, Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    painter->drawPixmap(yContent - 50, yMiddle + 5, copyLinkButton);

    painter->restore();
}

QSize GroupListDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &index) const
{
    return index.data(Qt::SizeHintRole).toSize();
}

GroupListView::GroupListView(QWidget *parent) :
    QListView(parent)
{
    setModel(new QStandardItemModel(this));
    setItemDelegate(new GroupListDelegate(this));
    setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    setAutoScroll(false);
    setDragEnabled(false);
    setEditTriggers(QListView::NoEditTriggers);
    setMouseTracking(true);

    connect(this, &GroupListView::clicked,
            this, &GroupListView::onItemClicked);
}

void GroupListView::mouseMoveEvent(QMouseEvent *event)
{
    QStandardItemModel *m = static_cast<QStandardItemModel *>(model());
    if (m)
    {
        QModelIndex index = indexAt(event->pos());

        if (index.isValid())
        {
            bool closeButtonTip = mapFromGlobal(QCursor::pos()).x() >= visualRect(index).width() - 40;
            bool copyLinkItem = mapFromGlobal(QCursor::pos()).x() >= visualRect(index).width() - 70 &&
                    mapFromGlobal(QCursor::pos()).x() <= visualRect(index).width() - 40;

            if (closeButtonTip)
                QToolTip::showText(QCursor::pos(), "Удалить из списка");

            if (copyLinkItem)
                QToolTip::showText(QCursor::pos(), "Скопировать ссылку группы");

            Qt::CursorShape shape = Qt::PointingHandCursor;
            setCursor(shape);
        }
        else
        {
            setCursor(Qt::ArrowCursor);
        }
    }
    QListView::mouseMoveEvent(event);
}

void GroupListView::onItemClicked(const QModelIndex &index)
{
    bool checked = index.data(Qt::CheckStateRole).toInt() == Qt::Checked;
    QStandardItem *item = static_cast<QStandardItemModel *>(model())->itemFromIndex(index);

    bool deleteItem = mapFromGlobal(QCursor::pos()).x() >= visualRect(index).width() - 40;
    bool copyLinkItem = mapFromGlobal(QCursor::pos()).x() >= visualRect(index).width() - 70 &&
            mapFromGlobal(QCursor::pos()).x() <= visualRect(index).width() - 40;

    if (copyLinkItem)
    {
        if( QClipboard* c = QApplication::clipboard() )
            c->setText("https://vk.com/" + index.data(GroupListDelegate::GROUP_SCREENNAME).toString());
        return;
    }

    if (deleteItem)
    {
        model()->removeRow(index.row());
        return;
    }
    
    if (!checked)
        item->setData(Qt::Checked, Qt::CheckStateRole);
    else
        item->setData(Qt::Unchecked, Qt::CheckStateRole);
}
