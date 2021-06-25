#ifndef FINISHEDLISTWIDGETITEMEDIT_H
#define FINISHEDLISTWIDGETITEMEDIT_H

#include <QWidget>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QMouseEvent>
#include <QEvent>

#include "fetcher.h"
#include "noerrorgroupitem.h"
#include "errorgroupitem.h"

namespace Ui {
class FinishedListWidgetItemEdit;
}

class FinishedListWidgetItemEdit : public QWidget
{
    Q_OBJECT
signals:
    void backToWaiting(MessagePack message);
    void backButtonReleased();

public:
    explicit FinishedListWidgetItemEdit(QWidget *parent = nullptr);
    ~FinishedListWidgetItemEdit();

    void setFetcher(const std::shared_ptr<Fetcher> fetcher);
    void setSendingResult(SendingResult result);

private slots:
    void onCancelButtonReleased();
    void onBackToWaitingButtonReleased();
    void onBackButtonReleased();

private:
    Ui::FinishedListWidgetItemEdit *ui;

    SendingResult mResult;
    std::shared_ptr<Fetcher> mFetcher = nullptr;
};

#endif // FINISHEDLISTWIDGETITEMEDIT_H
