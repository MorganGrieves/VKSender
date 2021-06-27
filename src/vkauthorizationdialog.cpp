#include "vkauthorizationdialog.h"
#include "ui_vkauthorizationdialog.h"

VkAuthorizationDialog::VkAuthorizationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VkAuthorizationDialog)
{
    ui->setupUi(this);

    setWindowFlags(Qt::WindowCloseButtonHint |
                   Qt::Dialog |
                   Qt::MSWindowsFixedSizeDialogHint);

    mVkAuthorizationView = new QWebView(this);
    connect(mVkAuthorizationView, &QWebView::urlChanged,
            this, &VkAuthorizationDialog::onViewUrlChanged);
}

VkAuthorizationDialog::~VkAuthorizationDialog()
{
    delete ui;
}

void VkAuthorizationDialog::showEvent(QShowEvent *event)
{
    mVkAuthorizationView->load(QUrl("https://oauth.vk.com/authorize?"
                   "client_id=7656391"
                   "&display=page"
                   "&redirect_uri=https://oauth.vk.com/blank.html"
                   "&scope=wall,photos,groups,offline"
                   "&response_type=token"
                   "&v=5.130"));

    qDebug() << "BrowserView Launched";
}

void VkAuthorizationDialog::onViewUrlChanged(const QUrl &url)
{
    qDebug() << "View Url Changed" << url.url();
    QRegExp reg("access_token=[\\d\\w]+");
    if (reg.indexIn(url.fragment()) != -1)
    {
        QString exm = reg.cap(0);
        QString *token = new QString (reg.cap(0).right(reg.cap(0).count() - reg.cap(0).indexOf("=") - 1));
        emit accessTokenReceived(token);
        close();
    }
}
