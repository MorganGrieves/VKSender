#ifndef LAUNCHEDLISTWIDGETITEM_H
#define LAUNCHEDLISTWIDGETITEM_H

#include <QWidget>

#include "fetcher.h"
#include "types.h"

namespace Ui {
class LaunchedListWidgetItem;
}

class LaunchedListWidgetItem : public QWidget
{
    Q_OBJECT

signals:
    void sendingFinished(SendingResult result);

public:
    explicit LaunchedListWidgetItem(QWidget *parent = nullptr);
    ~LaunchedListWidgetItem();

    void setFetcher(const std::shared_ptr<Fetcher> fetcher);
    void setMessagePackAndLaunch(const MessagePack &message);

private slots:
    void onAttachmentUpdated(QUuid id);

private:
    Ui::LaunchedListWidgetItem *ui;

    std::shared_ptr<Fetcher> mFetcher = nullptr;
    SendingResult mResult;
    MessagePack mMessage;

    size_t mOperationsAmount = 0;
};

#endif // LAUNCHEDLISTWIDGETITEM_H
