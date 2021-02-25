#ifndef SENDINGPROGRESS_H
#define SENDINGPROGRESS_H

#include <QDialog>

#include "repository.h"
#include "fetcher.h"
#include "types.h"

namespace Ui {
class SendingProgress;
}

class SendingProgress : public QDialog
{
    Q_OBJECT

public:
    explicit SendingProgress(QWidget *parent = nullptr);
    ~SendingProgress();
    void setRepository(const std::shared_ptr<Repository> repository);
    void setFetcher(const std::shared_ptr<Fetcher> fetcher);

signals:
    void messageSent(QString text, std::vector<Path> photoPaths);

public slots:
    void onMessageSent(QString text, std::vector<Path> photoPaths);

private:
    Ui::SendingProgress *ui;

    std::shared_ptr<Repository> mRepository = nullptr;
    std::shared_ptr<Fetcher> mFetcher = nullptr;

    std::size_t sentMessages = 0;
};

#endif // SENDINGPROGRESS_H
