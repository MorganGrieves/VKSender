#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QPixmap>
#include <QStyledItemDelegate>
#include <QWebView>

#include "waitingwidget.h"
#include "finishedwidget.h"
#include "launchedwidget.h"
#include "repository.h"
#include "greetingwidget.h"
#include "types.h"
#include "waitinglistwidgetitemedit.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class InterfaceListDelegate : public QAbstractItemDelegate
{
public:
    InterfaceListDelegate(QObject *parent = 0);

    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;

    virtual ~InterfaceListDelegate();

};

class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void messageSent(const QString messageText, const std::vector<Path> photoPaths);

public:

    enum Tabs
    {
        Waiting,
        Finished,
        Launched,
        Settings
    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onProfilePictureUpdated();
    void onUserNameUpdated();

    void onInterfaceListWidgetItemClicked(QListWidgetItem *item);

    void onChangeAccountButtonReleased();

private:
    void setInterfaceListWidget();
    QWidget *createNothingHereWidget();

    QPixmap roundPhoto100(QPixmap photo);
    void hideAllTabs();
    void setTabs();

private:
    Ui::MainWindow *ui;

    std::shared_ptr<Repository> mRepository = nullptr;
    std::shared_ptr<Fetcher> mFetcher = nullptr;
    std::shared_ptr<GreetingWidget> mGreetingWidget = nullptr;

    QWebView *mVkAuthorizationView = nullptr;

    QWidget *mNothingHereWidget = nullptr;
    WaitingWidget *mWaitingWidget = nullptr;
    FinishedWidget *mFinishedWidget = nullptr;
    LaunchedWidget *mLaunchedWidget = nullptr;

    bool mEditFormShowed = false;
};
#endif // MAINWINDOW_H
