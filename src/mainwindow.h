#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

#include "repository.h"
#include "grouptableedit.h"
#include "sendingprogress.h"
#include "greetingwidget.h"
#include "types.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void messageSent(const QString messageText, const std::vector<Path> photoPaths);

private slots:
    void onEditListButtonReleased();
    void onGroupDataUpdated();
    void onSendButtonReleased();
    void onAddNewPhotoButtonReleased();
    void onDeleteSelectedPhotosReleased();
    void onPhotosListWidgetDoubleClicked(const QModelIndex& index);

private:
    Ui::MainWindow *ui;

    std::shared_ptr<Repository> mRepository = nullptr;
    std::shared_ptr<Fetcher> mFetcher = nullptr;
    std::shared_ptr<GroupTableEdit> mGroupTableEdit = nullptr;
    std::shared_ptr<SendingProgress> mSendingProgress = nullptr;
    std::shared_ptr<GreetingWidget> mGreetingWidget = nullptr;
    std::map<Path, QListWidgetItem *> mPhotoPaths;
};
#endif // MAINWINDOW_H
