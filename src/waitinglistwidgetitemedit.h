#ifndef WAITINGLISTWIDGETITEMEDIT_H
#define WAITINGLISTWIDGETITEMEDIT_H

#include <QWidget>
#include <QMenu>
#include <QDebug>
#include <QListWidgetItem>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>

namespace Ui {
class WaitingListWidgetItemEdit;
}

class WaitingListWidgetItemEdit : public QWidget
{
    Q_OBJECT

signals:
    void backButtonReleased();
    void saveButtonReleased();
    void cancelButtonReleased();

public:
    explicit WaitingListWidgetItemEdit(QWidget *parent = nullptr);
    ~WaitingListWidgetItemEdit();

private slots:
    void onGroupListWidgetItemClicked(QListWidgetItem *item);
    void onBackButtonReleased();
    void onSaveButtonReleased();
    void onCancelButtonReleased();

private:
    void addUserGroupListWidgetItem();
    QPixmap roundPhoto75(QPixmap photo);

private:
    Ui::WaitingListWidgetItemEdit *ui;    
};

#endif // WAITINGLISTWIDGETITEMEDIT_H
