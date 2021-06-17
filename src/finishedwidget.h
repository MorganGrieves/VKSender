#ifndef FINISHEDWIDGET_H
#define FINISHEDWIDGET_H

#include <QWidget>

#include "fetcher.h"
#include "types.h"
#include "finishedlistwidgetitem.h"

namespace Ui {
class FinishedWidget;
}

class FinishedWidget : public QWidget
{
    Q_OBJECT

signals:
    void abortionFinished(MessagePack message);
    void finishedListWidgetItemReleased(FinishedListWidgetItemEdit *item);

public:
    explicit FinishedWidget(QWidget *parent = nullptr);
    ~FinishedWidget();

    void setFetcher(const std::shared_ptr<Fetcher> fetcher);
    void addFinishedItem(const SendingResult &result);

private:
    Ui::FinishedWidget *ui;

    std::shared_ptr<Fetcher> mFetcher = nullptr;
};

#endif // FINISHEDWIDGET_H
