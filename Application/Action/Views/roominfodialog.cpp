#include "roominfodialog.h"
#include "ui_roominfodialog.h"

RoomInfoDialog::RoomInfoDialog(RoomView *room) :
    QDialog(room, Qt::WindowSystemMenuHint | Qt::WindowTitleHint),
    ui(new Ui::RoomInfoDialog)
{
    if (room == NULL) return;

    this->room = room;
    ui->setupUi(this);
}

RoomInfoDialog::~RoomInfoDialog()
{
    delete ui;
}

/** Called to display the dialog to the user: */
void RoomInfoDialog::showDialog()
{
    refreshFields();
    show();
    activateWindow();
}

/** Update every field in the dialog: */
void RoomInfoDialog::refreshFields()
{
    RoomInfo *info = room->info();

    ui->addItemLb->setText( getPermissionString (
                                (RoomDescription::Permissions) info->getAddObjectPermission() ) );
    ui->banUsersLb->setText( getPermissionString (
                                (RoomDescription::Permissions) info->getBanPermission() ) );
    ui->changeTeamsLb->setText( getPermissionString (
                                (RoomDescription::Permissions) info->getChangeTeamsPermission() ) );
    ui->hideItemLb->setText( getPermissionString (
                                (RoomDescription::Permissions) info->getHideObjectPermission() ) );
    ui->lockItemLb->setText( getPermissionString (
                                (RoomDescription::Permissions) info->getLockObjectPermission() ) );
    ui->removeItemLb->setText( getPermissionString (
                                (RoomDescription::Permissions) info->getRemoveObjectPermission() ) );
    ui->teamSortingLb->setText( getTeamSortingTypeString(
                                (RoomInfo::TeamSortingType) info->getTeamSortingType() ));

    if (room->user()->isTheHost())
        info->setRoomIP( room->user()->getUserSettings()->getIp() );

    ui->addressLb->setText( info->getRoomIp() + " : " + QString::number(info->getRoomPort()) );

    ui->publishedLb->setText( (room->info()->IsRoomPublished())?tr("Yes"):tr("No") );

    ui->totalItemsLb->setText( QString::number(room->table()->totalOfItems()) );
    ui->roomNameLb->setText( info->getRoomName() );
    ui->typeLb->setText( info->getRoomType() );
    ui->hostUserLb->setText( room->hostUser()->getNickname() );
    ui->hostedSinceLb->setText( info->getCreationTime().toString() );
    QString description = info->getRoomDescription();
    if (description.isEmpty()) description = tr("No description");
    ui->descriptionLb->setText(description);

    if (room->user()->isTheHost()) {
        ui->maxUsersSpinBox->setVisible(true);
        ui->maxUsersLb->setVisible(false);
        ui->maxUsersSpinBox->setValue(info->getMaxNumberOfUsers());
    } else {
        ui->maxUsersSpinBox->setVisible(false);
        ui->maxUsersLb->setVisible(true);
        ui->maxUsersLb->setText( QString::number(info->getMaxNumberOfUsers()) );
    }
}


void RoomInfoDialog::on_maxUsersSpinBox_valueChanged(int)
{
    if (room->user()->isTheHost())
        room->info()->setMaxNumberOfUsers( ui->maxUsersSpinBox->value() );
}


/** Retranslates the app if its language changed: */
void RoomInfoDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        refreshFields();
    }
    QWidget::changeEvent(event);
}


QString RoomInfoDialog::getPermissionString(qint8 permission)
{
    switch(permission) {

    case RoomDescription::PERMISSION_HOST_ONLY:
        return tr("Host only");
        break;

    case RoomDescription::PERMISSION_ADMINS_ONLY:
        return tr("Admins only");
        break;

    case RoomDescription::PERMISSION_EVERYONE:
        return tr("Everyone");
        break;

    case RoomDescription::PERMISSION_NOBODY:
        return tr("Nobody");
        break;

    case RoomDescription::PERMISSION_YOURS_ONLY:
        return tr("Yours only");
        break;

    default :
        return tr("Invalid Permission");

    }
}

QString RoomInfoDialog::getTeamSortingTypeString(qint8 type)
{
    switch(type) {

    case RoomInfo::TEAM_ONE_PER_TEAM:
        return tr("One per team");
        break;

    case RoomInfo::TEAM_RANDOM_TEAMS:
        return tr("Random Teams");
        break;

    case RoomInfo::TEAM_SAME_TEAM:
        return tr("Same team");
        break;

    default:
        return tr("Team Sorting not identified");

    }
}
