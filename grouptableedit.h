#ifndef GROUPTABLEEDIT_H
#define GROUPTABLEEDIT_H

#include <QDialog>

#include "repository.h"
#include "fetcher.h"

namespace Ui {
class GroupTableEdit;
}

class GroupTableEdit : public QDialog
{
    Q_OBJECT

public:
    explicit GroupTableEdit(QWidget *parent = nullptr);
    ~GroupTableEdit();
    void setRepository(const std::shared_ptr<Repository> repository);
    void setFetcher(const std::shared_ptr<Fetcher> fetcher);

signals:
    void sendGroupLinks(std::vector<Link> links);

public slots:
    void onGroupVectorReceived(const std::vector<Group> groups);

private slots:
    void onSendButtonReleased();
    void onAddLinkButtonReleased();

private:
    void updateGroupsList();
    void addGroupFrame(Group group = Group());

private:
    Ui::GroupTableEdit *ui;

    std::shared_ptr<Repository> mRepository = nullptr;
    std::shared_ptr<Fetcher> mFetcher = nullptr;

    std::vector<Group> mGroup;
};

#endif // GROUPTABLEEDIT_H
