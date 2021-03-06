#include "greetingwidget.h"
#include "ui_greetingwidget.h"

#include <QWebView>
#include <QUrl>
#include <QRegExp>

GreetingWidget::GreetingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GreetingWidget)
{
    ui->setupUi(this);

    mVkAuthorizationView = new QWebView;

    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags | Qt::WindowStaysOnTopHint);

    connect (ui->launchButton, &QPushButton::released,
             this, &GreetingWidget::onLaunchButtonClicked);
    connect(mVkAuthorizationView, &QWebView::urlChanged,
            this, &GreetingWidget::onViewUrlChanged);
}

GreetingWidget::~GreetingWidget()
{
    delete ui;
    delete mVkAuthorizationView;
}

void GreetingWidget::onLaunchButtonClicked()
{
    mVkAuthorizationView->load(QUrl("https://oauth.vk.com/authorize?"
                   "client_id=7656391"
                   "&display=page"
                   "&redirect_uri=https://oauth.vk.com/blank.html"
                   "&scope=wall,photos,groups,offline"
                   "&response_type=token"
                   "&v=5.130"));

    mVkAuthorizationView->show();
}

void GreetingWidget::onViewUrlChanged(const QUrl &url)
{
    QRegExp reg("access_token=[\\d\\w]+");
    if (reg.indexIn(url.fragment()) != -1)
    {
        QString exm = reg.cap(0);
        QString token = reg.cap(0).right(reg.cap(0).count() - reg.cap(0).indexOf("=") - 1);
        emit accessTokenReceived(token);
        mVkAuthorizationView->close();
    }
}
