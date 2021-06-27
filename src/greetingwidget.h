#ifndef GREETINGWIDGET_H
#define GREETINGWIDGET_H

#include <QWidget>
#include <QWebView>
#include <QDialog>

#include "vkauthorizationdialog.h"

namespace Ui {
class GreetingWidget;
}

class GreetingWidget : public QWidget
{
    Q_OBJECT

signals:
    void accessTokenReceived(QString *token);

public:
    explicit GreetingWidget(QWidget *parent = nullptr);
    ~GreetingWidget();

private slots:
    void onLaunchButtonClicked();

private:
    Ui::GreetingWidget *ui;

    VkAuthorizationDialog *mVkAuthorizationDialog;
};

#endif // GREETINGWIDGET_H
