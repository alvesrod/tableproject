#include "loadingscreen.h"

LoadingScreen::LoadingScreen(Room *roomView, QWidget *parent) :
    QWidget(parent)
{
    /* Make sure the Engine is running: */
    if (!Engine::isRunning()) {
        qCritical() << "[ERROR] Engine class called, but Engine was not started!";
        return;
    }

    if (roomView == NULL) {
        qWarning() << "[LoadingScreen] Cannot initialize class without a Room View.";
        return;
    }

    loadingFailed = false;
    room = roomView;
    maxLoadingTime = DEFAULT_MAX_LOADING_TIME_SECONDS;
    connect(&loadingTimer, SIGNAL(timeout()), this, SLOT(loadingTimeout()));
}

void LoadingScreen::setRoomController(RoomController *controller)
{
    if (controller == NULL) {
        qWarning() << "[LoadingScreen] Room Controller is NULL.";
        return;
    }

    this->controller = controller;
}

/**
  StartLoading is called by the main window when it is time to load the room.
  */
void LoadingScreen::startLoading()
{
    loadingStep(LSTEP_ROOM_FOUND);
    connectToNetwork();
}

void LoadingScreen::setMaxLoadingTime(int seconds)
{
    maxLoadingTime = seconds;
}

void LoadingScreen::connectToNetwork()
{
    connect(controller, SIGNAL(errorMessage(QString)), this, SLOT(errorWarning(QString)));
    connect(controller, SIGNAL(connectedToServer(bool)), this, SLOT(connectedToServer(bool)));
    connect(controller, SIGNAL(roomPrepared(bool,QString)), this, SLOT(roomPrepared(bool,QString)));

    if (controller->isHost()) {
        //NEW HOST:
        connect(controller, SIGNAL(serverStarted(bool)), this, SLOT(serverStarted(bool)));
        controller->startNewServer();
    }
    else
        controller->connectToServer();
}

void LoadingScreen::loadingStep(LoadingScreen::LoadingSteps step)
{
    if (loadingFailed) return;
    loadingTimer.stop();
    loadingStepComplete(step);
    loadingTimer.start(maxLoadingTime*1000);
}

void LoadingScreen::connectedToServer(bool success)
{
    if (loadingFailed) return;
    if ( success ) {
        loadingStep(LSTEP_ROOM_CONNECTED);

        /*
         * The room might want to do some connections with the RoomController
         * before asking to prepare room:
         */
        if (!room->prepareRoom()) {
            error(tr("The room could not be prepared."));
            return;
        }

        controller->sendMessageToServer(RoomController::MSG_PREPARE_ROOM);
    }
     else
        error(tr("Could not connect to the server"));
}

void LoadingScreen::serverStarted(bool success)
{
    if (loadingFailed) return;
    if ( success ) {
        loadingStep(LSTEP_ROOM_HOSTED);
        controller->connectToServer();
    }
     else
        error(tr("Could not start a server"));
}

void LoadingScreen::roomPrepared(bool success, QString result)
{
    if (loadingFailed) return;
    if (success) {
        loadingStep(LSTEP_ROOM_PREPARED);

        /* Try to load the room. The room already has a pointer to the room controller: */
        if (!room->loadRoom()) {
            error(tr("The room could not be loaded."));
            return;
        }
        loadingStep(LSTEP_ROOM_LOADED);

        connect(controller, SIGNAL(someoneConnectedToServer(User*)), this, SLOT(connectedToRoom(User*)), Qt::QueuedConnection);
        controller->sendMessageToServer(RoomController::MSG_ROOM_PREPARED);
    }
    else {
        error(tr("Connection refused: ") + result);
    }
}

void LoadingScreen::connectedToRoom(User *user)
{
    if (loadingFailed) return;
    /* We are only interested if you connected, not others: */
    if (user != controller->getCurrentUser()) return;

    loadingStep(LSTEP_ROOM_JOINED);

    if (controller->isHost()) {
        /* Ask the WebContact to display this room into the public list */
        connect(this, SIGNAL(publishRoom(qint32)), WebContact::getInstance(), SLOT(publishRoom(qint32)), Qt::QueuedConnection);
        emit publishRoom( controller->getRoomDescription()->getRoomPort() );
    }

    /* Ok, you are connected. So, it's time to open the room: */
    emit openRoomView();
}

void LoadingScreen::loadingTimeout()
{
    error(tr("Time expired: The room could not be loaded."));
}

void LoadingScreen::error(QString error)
{
    if (loadingFailed) return;
    loadingFailed = true;
    loadingTimer.stop();
    controller->leaveRoom();
    errorWarning(error);
}

