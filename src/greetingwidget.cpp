#include "greetingwidget.h"
#include "ui_greetingwidget.h"

#include <QUrl>
#include <QRegExp>

GreetingWidget::GreetingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GreetingWidget)
{
    ui->setupUi(this);

    mVkAuthorizationDialog = new VkAuthorizationDialog(this);

    connect (ui->launchButton, &QPushButton::released,
             this, &GreetingWidget::onLaunchButtonClicked);
    connect(mVkAuthorizationDialog, &VkAuthorizationDialog::accessTokenReceived,
            this, &GreetingWidget::accessTokenReceived);
}

GreetingWidget::~GreetingWidget()
{
    delete ui;
}

void GreetingWidget::onLaunchButtonClicked()
{
    mVkAuthorizationDialog->exec();
}
