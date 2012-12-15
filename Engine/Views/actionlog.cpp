#include "actionlog.h"
#include "ui_actionlog.h"

#define TOTAL_ITEMS 50

ActionLog::ActionLog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ActionLog)
{
    ui->setupUi(this);
}

ActionLog::~ActionLog()
{
    delete ui;
}

void ActionLog::actionLog(QListWidgetItem *item)
{
    ui->logWidget->addItem(item);
    if (ui->logWidget->count() > TOTAL_ITEMS)
        ui->logWidget->takeItem(0);
    ui->logWidget->scrollToBottom();
}

void ActionLog::on_logWidget_activated(const QModelIndex &index)
{
    emit selected(index);
}


/** Retranslates the app if its language changed: */
void ActionLog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
