#ifndef GREETINGWIDGET_H
#define GREETINGWIDGET_H

#include <QWidget>
#include <QWebView>

namespace Ui {
class GreetingWidget;
}

class GreetingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GreetingWidget(QWidget *parent = nullptr);
    ~GreetingWidget();

signals:
    void accessTokenReceived(QString token);

private slots:
    void onLaunchButtonClicked();
    void onViewUrlChanged(const QUrl &url);

private:
    Ui::GreetingWidget *ui;

    QWebView * mVkAuthorizationView;
};

#endif // GREETINGWIDGET_H
