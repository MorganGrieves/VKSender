#ifndef LAUNCHEDWIDGET_H
#define LAUNCHEDWIDGET_H

#include <QWidget>

#include "launchedlistwidgetitem.h"
#include "fetcher.h"
#include "types.h"

namespace Ui {
class LaunchedWidget;
}

class LaunchedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LaunchedWidget(QWidget *parent = nullptr);
    ~LaunchedWidget();

    void setFetcher(const std::shared_ptr<Fetcher> fecther);
    void addLaunchedItem(const MessagePack &pack);

private:
    Ui::LaunchedWidget *ui;

    std::vector<LaunchedListWidgetItem *> mLaunchedListItemList;
    std::shared_ptr<Fetcher> mFetcher = nullptr;
};

#endif // LAUNCHEDWIDGET_H
