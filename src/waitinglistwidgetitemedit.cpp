#include "waitinglistwidgetitemedit.h"
#include "ui_waitinglistwidgetitemedit.h"

WaitingListWidgetItemEdit::WaitingListWidgetItemEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaitingListWidgetItemEdit)
{
    ui->setupUi(this);
}

WaitingListWidgetItemEdit::~WaitingListWidgetItemEdit()
{
    delete ui;
}
