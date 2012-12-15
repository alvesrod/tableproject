#include "lockhidedialog.h"
#include "ui_lockhidedialog.h"

LockHideDialog::LockHideDialog(RoomView *room) :
    QDialog(room),
    ui(new Ui::LockHideDialog)
{
    if (room == NULL) return;
    this->room = room;

    ui->setupUi(this);
    isLockDialog = false;
    item = NULL;

    setWindowFlags(Qt::Tool);

    /* Prepare list of teams: */
    makeStringListOfTeams();

}

LockHideDialog::~LockHideDialog()
{
    delete ui;
}

void LockHideDialog::openLockDialog(TableItem *item)
{
    this->item = item;
    isLockDialog = true;
    teams = item->getLockedTeams();
    setLockDialog();
    loadDialog();
}

void LockHideDialog::openHideDialog(TableItem *item)
{
    if (item == NULL) return;
    this->item = item;
    isLockDialog = false;
    teams = item->getHiddenTeams();
    setHideDialog();
    loadDialog();
}

void LockHideDialog::sendAndClose()
{
    if (isLockDialog)
        emit lockItems(item, teams);
    else
        emit hideItems(item, teams);

    close();
}


void LockHideDialog::on_OkButtonBox_accepted()
{
    makeListOfTeams();
    sendAndClose();
}

void LockHideDialog::on_genericBtn1_clicked() //Lock or Hide to others
{
    makeListOfOtherTeams();
    sendAndClose();
}

void LockHideDialog::on_genericBtn2_clicked() //Unlock or show to all
{
    if (!teams.isEmpty()) teams.clear();
    sendAndClose();
}


void LockHideDialog::loadDialog()
{
    updateScreenText();
    bool allSelected = true;
    for (qint8 i = 1; i <= teamString.size(); i++) {
        if (teams.contains(i-1))
            ui->listWidget->item(i)->setCheckState(Qt::Checked);
        else {
            ui->listWidget->item(i)->setCheckState(Qt::Unchecked);
            allSelected = false;
        }
    }
    if (allSelected)
        ui->listWidget->item(0)->setCheckState(Qt::Checked);
    else
        ui->listWidget->item(0)->setCheckState(Qt::Unchecked);

    show();
    activateWindow();
}

void LockHideDialog::makeStringListOfTeams()
{
    ui->listWidget->clear();
    teamString = RoomMember::getTeamList();
    ui->listWidget->addItem("");
    for (qint8 i = 1; i <= teamString.size(); i++) {
        ui->listWidget->addItem(teamString[i-1]);
        ui->listWidget->item(i)->setFlags(Qt::ItemIsEnabled |Qt::ItemIsUserCheckable);
    }
}

void LockHideDialog::makeListOfTeams()
{
    if (!teams.isEmpty()) teams.clear();
    for (qint8 i = 1; i <= teamString.size(); i++) {
        if ( ui->listWidget->item(i)->checkState() )
            teams << i-1;
    }
}

void LockHideDialog::makeListOfOtherTeams()
{
    if (!teams.isEmpty()) teams.clear();
    for (qint8 i = 0; i < teamString.size(); i++) {
        if (room->user()->getTeam() != i ) //If its not your team
            teams << i;
    }
}

void LockHideDialog::setHideDialog()
{
    ui->titleLabel->setText(tr("Hide") + "<b> " + item->getName() + "</b> " + tr("from:"));
    ui->genericBtn1->setText(tr("Hide to others"));
    ui->genericBtn2->setText(tr("Show to all"));
}

void LockHideDialog::setLockDialog()
{
    ui->titleLabel->setText(tr("Lock") + "<b> " + item->getName() + "</b> " + tr("to:"));
    ui->genericBtn1->setText(tr("Lock to others"));
    ui->genericBtn2->setText(tr("Unlock to all"));
}

void LockHideDialog::updateScreenText()
{
    teamString = RoomMember::getTeamList();
    ui->listWidget->item(0)->setText(tr("Select All"));
    for (qint8 i = 1; i <= teamString.size(); i++) {
        bool yourTeam = (room->user()->getTeam() == i-1);
        ui->listWidget->item(i)->setText(teamString[i-1] + ( (yourTeam) ? tr(" (your team)") : "") );
    }
    if (isLockDialog)
        setLockDialog();
    else
        setHideDialog();
}



/** Retranslates the app if its language changed: */
void LockHideDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        /* Update teams: */
        if (item != NULL)
            updateScreenText();
    }
    QDialog::changeEvent(event);
}

void LockHideDialog::on_OkButtonBox_rejected()
{
    close();
}


void LockHideDialog::on_listWidget_itemClicked(QListWidgetItem *item)
{
    /* Select all of deselect all if the first item was pressed: */
    qDebug() << "ACTIVATE: " << item;
    if (item == ui->listWidget->item(0)) {
        for (qint8 i = 1; i <= teamString.size(); i++) {
            if ( ui->listWidget->item(0)->checkState() )
                ui->listWidget->item(i)->setCheckState(Qt::Checked);
            else
                ui->listWidget->item(i)->setCheckState(Qt::Unchecked);
        }
    }
}
