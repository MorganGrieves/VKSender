#include "grouptableedit.h"
#include "ui_grouptableedit.h"

#include <QLineEdit>
#include <QObject>

GroupTableEdit::GroupTableEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GroupTableEdit)
{
    ui->setupUi(this);

    connect(ui->addLinkButton, &QPushButton::released,
            this, &GroupTableEdit::onAddLinkButtonReleased);

    connect(ui->sendGroupListButton, &QPushButton::released,
            this, &GroupTableEdit::onSendButtonReleased);

    connect(ui->deleteAllLinkButton, &QPushButton::released,
            [this]()
            {
                ui->linksBeforeSendList->clear();
                mGroups.clear();
                qDebug() << "all items were deleted";
            });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [=]()
    {
        qDebug() << "QDialogButtonBox accepted";
        mRepository->setGroupData(mGroups);
        this->close();
    });

    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, [=]()
    {
        qDebug() << "QDialogButtonBox rejected";
        this->close();
    });

    connect(ui->linksBeforeSendList, &QListWidget::itemDoubleClicked,
            [this](QListWidgetItem *item)
    {
        qDebug() << ui->linksBeforeSendList->itemWidget(item)->findChild<QLineEdit *>()->text();
        mGroups.erase(std::remove_if(mGroups.begin(), mGroups.end(),
                                  [this, item](Group group)
        {
                          return ui->linksBeforeSendList->itemWidget(item)->findChild<QLineEdit *>()->text()
                                  == group.vkid;

                      }),
                   mGroups.end()
                );
        delete item;
        qDebug() << "item was deleted";
    });
}

void GroupTableEdit::setFetcher(const std::shared_ptr<Fetcher> fetcher)
{
    mFetcher = fetcher;

    connect(this, &GroupTableEdit::sendGroupLinks,
            mFetcher.get(), &Fetcher::onGroupDataNeed);
}

void GroupTableEdit::setRepository(const std::shared_ptr<Repository> repository)
{
    mRepository = repository;

    for (const auto &group : mRepository->getGroupData())
        addGroupFrame(group);
}

void GroupTableEdit::addGroupFrame(Group group)
{
    QFrame *groupFrame = new QFrame(ui->linksBeforeSendList);
    groupFrame->setStyleSheet("border: 1px blue;"
                              "background-color: red;");

    QLayout *groupFrameLayout = new QHBoxLayout();

    QLabel *vkLinkLabel = new QLabel("https://vk.com/");
    vkLinkLabel->setAlignment(Qt::AlignRight);
    vkLinkLabel->setStyleSheet(//"background-color: white;"
                               "margin: 5px;"
                               "color:#656565;");

    QLabel *vkNameLabel = new QLabel("");
    vkNameLabel->setObjectName("groupNameLabel");
    vkNameLabel->setAlignment(Qt::AlignRight);
    vkNameLabel->setStyleSheet(//"background-color: white;"
                               "margin: 5px;"
                               "color:blue;");

    QLineEdit *linkLineEdit = new QLineEdit;
    linkLineEdit->setStyleSheet("border: 1px solid black;");
    if (!group.vkid.isEmpty())
        linkLineEdit->setText(group.vkid);

    groupFrameLayout->addWidget(vkLinkLabel);
    groupFrameLayout->addWidget(linkLineEdit);
    groupFrameLayout->addWidget(vkNameLabel);
    groupFrame->setLayout(groupFrameLayout);

    QListWidgetItem* item = new QListWidgetItem( ui->linksBeforeSendList );
    item->setSizeHint( groupFrame->sizeHint() );
    ui->linksBeforeSendList->setItemWidget( item, groupFrame );

    qDebug() << "groupFrame was added";
}

void GroupTableEdit::onAddLinkButtonReleased()
{
    addGroupFrame();
}

void GroupTableEdit::onSendButtonReleased()
{
    std::vector<Link> groupLinks;
    const auto links = ui->linksBeforeSendList->findChildren<QLineEdit *>();
    for (const auto &link : links)
        groupLinks.push_back(link->text());
    connect (mFetcher.get(), &Fetcher::updatedGroupData,
             this, &GroupTableEdit::onGroupVectorReceived);
    emit sendGroupLinks(groupLinks);
}

void GroupTableEdit::onGroupVectorReceived(const QVector<Group> groups)
{
    mGroups.clear();
    const auto links = ui->linksBeforeSendList->findChildren<QLineEdit *>();

    for (const auto &link : links)
        for (const auto &group : groups)
        {
            QFrame *groupFrame = qobject_cast< QFrame* >(link->parent());
            if (link->text() == group.link || link->text() == group.vkid)
            {
                groupFrame->setStyleSheet("background-color: green;");
                mGroups.push_back(group);

                groupFrame->findChild<QLabel*>("groupNameLabel")->setText(group.name);
            }
        }
}

GroupTableEdit::~GroupTableEdit()
{
    delete ui;
}
