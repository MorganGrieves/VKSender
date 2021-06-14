#ifndef FINISHEDWIDGET_H
#define FINISHEDWIDGET_H

#include <QWidget>

#include "fetcher.h"
#include "types.h"

namespace Ui {
class FinishedWidget;
}

class FinishedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FinishedWidget(QWidget *parent = nullptr);
    ~FinishedWidget();

    void setFetcher(const std::shared_ptr<Fetcher> fetcher);

private:
    Ui::FinishedWidget *ui;

    std::shared_ptr<Fetcher> mFetcher = nullptr;
};

#endif // FINISHEDWIDGET_H
