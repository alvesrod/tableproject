#include "newroom.h"

NewRoom::NewRoom(UserSettings *settings, QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    if (settings == NULL) {
        qWarning() << "Cannot initialize NewRoom without User Settings.";
        return;
    }
    userSettings = settings;
    nicknameLb = NULL;
    connect(userSettings, SIGNAL(nicknameChanged(QString)), this, SLOT(nicknameChanged(QString)));
}

void NewRoom::nicknameChanged(QString name)
{
    nicknameLb->setText(name);
}

void NewRoom::openUserSettingsDialog()
{
    userSettings->openDialog();
}

void NewRoom::refreshUserNicknameLabel()
{
    nicknameLb->setText( userSettings->getNickname() );
}

void NewRoom::setupNewRoomUI()
{
    linkViewComponents(nicknameLb);
    refreshUserNicknameLabel();
}

void NewRoom::startNewRoom(RoomDescription *roomDescription)
{
    RoomController *roomController = new RoomController( roomDescription, userSettings );
    this->close();
    emit hostRoom(roomController);
}

QString NewRoom::getUserLocation()
{
    return userSettings->getLocation();
}
