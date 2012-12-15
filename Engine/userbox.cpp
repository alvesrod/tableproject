#include "userbox.h"

UserBox::UserBox(User *user, QWidget *parent) :
    QWidget(parent)
{
    /* Make sure the Engine is running: */
    if (!Engine::isRunning()) {
        qCritical() << "[ERROR] Engine class called, but Engine was not started!";
        return;
    }

    if (user == NULL) {
        qCritical() << "[UserBox] Cannot initialize class without a valid user";
        return;
    }
    this->user = user;
    connect(user, SIGNAL(nicknameChanged(User*,User*,QString)), this, SLOT(nicknameUpdateFromUser(User*,User*,QString)));
    connect(user, SIGNAL(adminStatusChanged(User*,User*)), this, SLOT(adminStatusUpdateFromUser(User*,User*)));

    roomController = NULL;

    latencyUpdateTimer = new QTimer(this);
    connect(latencyUpdateTimer, SIGNAL(timeout()), this, SLOT(refreshLatency()));

}

UserBox::~UserBox()
{
    qDebug() << "UserBox destroyed";
}

User *UserBox::getUser()
{
    return user;
}

void UserBox::setRoomController(RoomController *controller)
{
    if (controller == NULL) {
        qWarning() << "[UserBox] Error: Room Controller is null.";
        return;
    }

    roomController = controller;
    connect( roomController, SIGNAL(pingReceived(User*)), this, SLOT(pingReceived(User*)));
    connect( roomController->getCurrentUser(), SIGNAL(adminStatusChanged(User*,User*)),
             this, SLOT(adminStatusChanged(User*,User*)));
}

void UserBox::userLeft()
{
    on_user_left();
}

void UserBox::refreshLatency()
{
    if ( (user == NULL) || (roomController == NULL) ) {
        latencyUpdateTimer->stop();
        return;
    }
    user->latencyRequested();
    roomController->sendMessageToServer(RoomController::MSG_PING, 0, user);
}

RoomDescription *UserBox::getRoomDescription()
{
    if (roomController == NULL) {
        qCritical() << "[UserBox] ERROR: Room Controller is NULL!";
        return NULL;
    }
    return roomController->getRoomDescription();
}

User *UserBox::getCurrentUser()
{
    return roomController->getCurrentUser();
}

void UserBox::setAdminStatus(bool isAdmin)
{
    if (isAdmin == user->isAnAdmin()) return;
    user->setIsAdmin(isAdmin);
    user->sendEditUserMessage(User::MSG_CHANGE_ADMIN_STATUS);
}

void UserBox::pingReceived(User *user)
{
    if (user != this->user) return;
    displayLatency(user->getLatency());
}

void UserBox::nicknameUpdateFromUser(User *userChanged, User *, QString)
{
    if (userChanged != user) return;
    refreshUI();
}

void UserBox::adminStatusUpdateFromUser(User *userChanged, User *)
{
    if (userChanged != user) return;
    refreshUI();
}

/** When YOUR admin status changed (which could cause permission changes): */
void UserBox::adminStatusChanged(User *userChanged, User *)
{
    if (userChanged != roomController->getCurrentUser()) return;
    refreshUI();
}

void UserBox::constantLatencyUpdate()
{
    latencyUpdateTimer->start(LATENCY_UPDATE_INTERVAL_SECONDS*1000);
}

bool UserBox::addChatMessage(QString message, QColor color)
{
    if (roomController == NULL) return false;
    return roomController->addChatMessage(message, color);
}

void UserBox::showEvent(QShowEvent *)
{
    if (roomController == NULL) {
        qCritical() << "[UserBox] ERROR: Cannot show user box without a controller.";
        return;
    }

    refreshUI();
    constantLatencyUpdate();
    refreshLatency();
}

void UserBox::hideEvent(QHideEvent *)
{
    latencyUpdateTimer->stop();
}



