#ifndef WAITINGWIDGET_H
#define WAITINGWIDGET_H

#include <QWidget>

namespace Ui {
class WaitingWidget;
}

class WaitingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WaitingWidget(QWidget *parent = nullptr);
    ~WaitingWidget();

private:
    Ui::WaitingWidget *ui;
};

#endif // WAITINGWIDGET_H
