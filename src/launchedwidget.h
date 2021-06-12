#ifndef LAUNCHEDWIDGET_H
#define LAUNCHEDWIDGET_H

#include <QWidget>

namespace Ui {
class LaunchedWidget;
}

class LaunchedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LaunchedWidget(QWidget *parent = nullptr);
    ~LaunchedWidget();

private:
    Ui::LaunchedWidget *ui;
};

#endif // LAUNCHEDWIDGET_H
