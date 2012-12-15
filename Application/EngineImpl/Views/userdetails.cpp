#include "userdetails.h"
#include "ui_userdetails.h"

UserDetails::UserDetails(User *user, QWidget *parent) :
UserBox(user, parent), ui(new Ui::UserDetails)
{
    if (user == NULL) {
        qWarning() << "Cannot initialize UserDetails without a valid member.";
        return;
    }

    /*
     * Typecast to the class that extends User
     * (because it has more specific methods outside the Engine):
     */
    member = dynamic_cast<RoomMember*> (user);

    /* Subscribe to team updates, so that the UI can reflect the changes: */
    connect(member, SIGNAL (teamChanged(RoomMember*,RoomMember*)),
              this, SLOT   (teamUpdateFromUser(RoomMember*,RoomMember*)) );

    /* Display the view: */
    ui->setupUi(this);

    /* Display all the possible teams in the combo box: */
    setUpTeamComboBox();
}

UserDetails::~UserDetails()
{
    delete ui;
}

void UserDetails::displayLatency(int latency)
{
    ui->latencyLb->setText(QString::number(latency) + " ms");
}

void UserDetails::on_user_left()
{
    updateOnlineLb();
    ui->teamComboBox->setEnabled(false);
    ui->isAdminCheckBox->setEnabled(false);
}

void UserDetails::setUpTeamComboBox()
{
    ui->teamComboBox->setEnabled(false);
    ui->teamComboBox->clear();
    ui->teamComboBox->addItems( member->getTeamList() );
    ui->teamComboBox->setCurrentIndex( member->getTeam() );
}

/**
 * @return true if the user viewing this widget has permission to change
 * the team of the widget user.
 */
bool UserDetails::getChangeTeamPermission()
{
    RoomInfo* info = dynamic_cast<RoomInfo*>(getRoomDescription());
    RoomMember *user = dynamic_cast<RoomMember*>(getCurrentUser());
    return Permission::canChangeTeam(user, info, (getCurrentUser() == getUser()));
}

/**
 * @brief Prepare the permissions for every component in the
 * view that might behave differently depending on who is viewing it.
 */
void UserDetails::setUpPermissions()
{
    ui->teamComboBox->setEnabled(getChangeTeamPermission());

    /* Only the host can make admins: */
    ui->isAdminCheckBox->setEnabled( getCurrentUser()->isTheHost() );
    ui->isAdminCheckBox->setChecked( member->isAnAdmin() );
}

/** Called when the index in the team combo box was changed: */
void UserDetails::on_teamComboBox_currentIndexChanged(int index)
{
    if (!ui->teamComboBox->isEnabled())
        return; //since the box was disabled, it was not changed by you, so ignore.
    changeTeam(index);
}

void UserDetails::changeTeam(int team, bool submitChanges)
{
    if (team == member->getTeam()) return;
    ui->teamComboBox->setEnabled(false);
    ui->teamComboBox->setCurrentIndex(team);
    member->setTeam( (RoomMember::TeamColor) team );
    if (submitChanges)
        member->sendEditUserMessage(RoomMember::MSG_CHANGE_TEAM);
    /*
     * No need to enable the comboBox again because it will be enabled
     * when the message arrives (since it was sent to everybody).
     * This is actually good to avoid spamming. It's better to wait for
     * the message to arrive before changing the team again.
     */
}

/** Retranslates the app if its language changed: */
void UserDetails::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        setUpTeamComboBox();
        refreshUI();
    }
    QWidget::changeEvent(event);
}

/** Called by the superclass when something changed in the view (ask to refresh it): */
void UserDetails::refreshUI()
{
    ui->usernameLb->setText(member->getNickname()); //refresh the nickname
    changeTeam( member->getTeam(), false ); //@param false: don't submit changes since you didn't change it manually.
    setUpPermissions();
    updateOnlineLb();
}

void UserDetails::on_isAdminCheckBox_clicked(bool checked)
{
    setAdminStatus(checked);
}

/** When the user this object represents changed teams (message from the server): */
void UserDetails::teamUpdateFromUser(RoomMember *userChanged, RoomMember *)
{
    if (userChanged != member) return;

    if (member->getTeam() != ui->teamComboBox->currentIndex()) {
        ui->teamComboBox->setEnabled(false); //Disable, so that changes won't be sent back.
        ui->teamComboBox->setCurrentIndex( member->getTeam() );
    }
    ui->teamComboBox->setEnabled(getChangeTeamPermission());
}

/** Display whether or not the user is online: */
void UserDetails::updateOnlineLb()
{
    ui->onlineLb->setText( (member->isUserOnline()) ? tr("Yes") : tr("No")  );
}


