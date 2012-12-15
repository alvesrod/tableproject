#include "userbuttonbox.h"
#include "ui_userbuttonbox.h"

UserButtonBox::UserButtonBox(User *boxUser, User *currentUser, RoomDescription *description, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserButtonBox)
{
    if ( (boxUser == NULL) || (currentUser == NULL) ) {
        qWarning() << "Cannot initialize UserButtonBox without a valid user.";
        return;
    }

    if (description == NULL) {
        qWarning() << "Cannot initialize UserButtonBox without a valid room description.";
        return;
    }

    user = boxUser;
    this->currentUser = currentUser;
    roomDescription = description;

    ui->setupUi(this);

    /* The user can talk privately with everyone but himself: */
    ui->privateMessageBtn->setEnabled( !user->isThatYou() );

    /* Enable the ban button if the user has permission to ban others: */
    updateBanButtonPermission();
}

UserButtonBox::~UserButtonBox()
{
    delete ui;
}

User *UserButtonBox::getUser()
{
    return user;
}

void UserButtonBox::updateBanButtonPermission()
{
    ui->banBtn->setEnabled(getBanPermission());
}

void UserButtonBox::on_privateMessageBtn_clicked()
{
    QSound::play("Sound/menuClick.wav");
    emit privateMsgBtnClicked(user);
}

void UserButtonBox::on_moreBtn_clicked()
{
    emit moreBtnClicked(user);
}

/** Retranslates the app if its language changed: */
void UserButtonBox::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
    QWidget::changeEvent(event);
}

void UserButtonBox::on_banBtn_clicked()
{
    QSound::play("Sound/menuClick.wav");
    banUser();
}

bool UserButtonBox::getBanPermission()
{
    bool permission = false;

    if ((!user->isThatYou()) && (!user->isTheHost())) //Can't ban the host or yourself.
        permission = currentUser->hasPermission(roomDescription->getBanPermission());

    return permission;
}

void UserButtonBox::banUser()
{
    /* Ask if the user really wants to ban the user: */
    int reply = QMessageBox::question(this, tr("Banning user"),
                                      tr("Are you sure you want to ban") + " <b>" + user->getNickname() + "</b>?",
                                                              QMessageBox::Yes, QMessageBox::No);
    if (reply == QMessageBox::Yes)
        emit banUserBtnClicked(user);
}
