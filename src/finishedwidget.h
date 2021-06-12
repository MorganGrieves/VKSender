#ifndef FINISHEDWIDGET_H
#define FINISHEDWIDGET_H

#include <QWidget>

namespace Ui {
class FinishedWidget;
}

class FinishedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FinishedWidget(QWidget *parent = nullptr);
    ~FinishedWidget();

private:
    Ui::FinishedWidget *ui;
};

#endif // FINISHEDWIDGET_H
