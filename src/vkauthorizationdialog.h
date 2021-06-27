#ifndef VKAUTHORIZATIONDIALOG_H
#define VKAUTHORIZATIONDIALOG_H

#include <QDialog>
#include <QWebView>

#include "types.h"

namespace Ui {
class VkAuthorizationDialog;
}

class VkAuthorizationDialog : public QDialog
{
    Q_OBJECT

signals:
    void accessTokenReceived(QString *);

public:
    explicit VkAuthorizationDialog(QWidget *parent = nullptr);
    ~VkAuthorizationDialog();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onViewUrlChanged(const QUrl &url);

private:
    Ui::VkAuthorizationDialog *ui;

    QWebView *mVkAuthorizationView;
};

#endif // VKAUTHORIZATIONDIALOG_H
