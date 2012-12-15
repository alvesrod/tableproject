#include "room.h"

Room::Room(QWidget *parent) :
    QWidget(parent)
{
    /* Make sure the Engine is running: */
    if (!Engine::isRunning()) {
        qCritical() << "[ERROR] Engine class called, but Engine was not started!";
        return;
    }
    fileTransfer = NULL;

    hasLeftRoom = false; //true after the user leaves the room
    isUserInRoom = false; //true when the room is shown
    roomLoaded = false; //true after loadRoom() is called.

    /*
     * Constructor is called together with the loading screen. So,
     * this should not be heavy. All the heavy data should be done
     * in the loadRoom() method.
     */
}

void Room::setRoomController(RoomController *controller)
{
    if (controller == NULL) {
        qWarning() << "[Room] Error: Room Controller is NULL!";
        return;
    }

    if (fileTransfer)
        fileTransfer->exit(); //This thread is deleted when exited.

    roomController = controller;
}

/** This function is called by the loading screen before data has arrived: */
bool Room::prepareRoom()
{
    connect( roomController, SIGNAL(disconnectedFromServer(bool)), this, SLOT(disconnectedFromServer(bool)));
    connect( roomController, SIGNAL(serverClosed(bool)), this, SLOT(serverClosed(bool)));
    connect( roomController, SIGNAL(leftRoom()), this, SLOT(leftRoom()));
    connect( roomController, SIGNAL(banned()), this, SLOT(banned()));
    connect( roomController, SIGNAL(actionMessageReceived(QByteArray,User*)), this, SLOT(messageReceived(QByteArray,User*)));
    connect( roomController, SIGNAL(someoneConnectedToServer(User*)), this, SLOT(someoneConnected(User*)));
    connect( roomController, SIGNAL(someoneDisconnectedFromServer(User*)), this, SLOT(someoneDisconnected(User*)));
    connect( roomController, SIGNAL(randomValuesReceived(qint32,qint32,qint16,QList<qint32> ,User*)),
             this, SLOT  (randomReceived      (qint32,qint32,qint16,QList<qint32> ,User*)) );

    return on_loadingScreen_prepare();
}

/** This function is called by the loading screen asking to load room after data has arrived: */
bool Room::loadRoom()
{
    fileTransfer = new FileTransfer(roomController);

    connect( fileTransfer, SIGNAL( started                  (QByteArray,qint32,User*)),
                     this, SLOT  ( largeMessageStarted      (QByteArray,qint32,User*)), Qt::QueuedConnection);
    connect( fileTransfer, SIGNAL( downloadProgress         (QByteArray,double,User*)),
                     this, SLOT  ( largeMessageProgress     (QByteArray,double,User*)), Qt::QueuedConnection);
    connect( fileTransfer, SIGNAL( finished                 (QByteArray,QByteArray,User*)),
                     this, SLOT  ( largeMessageFinished     (QByteArray,QByteArray,User*)), Qt::QueuedConnection);
    connect( fileTransfer, SIGNAL( cancelled                (QByteArray,User*)),
                     this, SLOT  ( largeMessageCancelled    (QByteArray,User*)), Qt::QueuedConnection);
    connect(         this, SIGNAL(sendP                     (QByteArray,QByteArray,User*)),
             fileTransfer, SLOT  (sendPackage               (QByteArray,QByteArray,User*)), Qt::QueuedConnection);
    connect(         this, SIGNAL(sendF                     (QString,QByteArray,User*)),
             fileTransfer, SLOT  (sendFile                  (QString,QByteArray,User*)), Qt::QueuedConnection);

    /* Run thread with low priority: */
    fileTransfer->start(QThread::LowPriority);

    roomLoaded = on_loadingScreen_load();
    return roomLoaded;
}

/** This function is called by the mainMenu when the room is loaded: */
bool Room::startRoom() {
    isUserInRoom = true;
    return on_room_loaded();
}

User *Room::getHostUser()
{
    if (roomController == NULL) return NULL;
    return roomController->getHostUser();
}

bool Room::addChatMessage(QString message, QColor color)
{
    if (roomController == NULL) return false;
    return roomController->addChatMessage(message, color);
}

void Room::disconnectedFromServer(bool isDisconnected)
{
    if (isDisconnected)
        leaveRoom(MSG_DISCONNECTED);
}

void Room::messageReceived(QByteArray package, User *user)
{
    /* Put the byte array inside a QDataStream, so that the message can be opened: */
    QDataStream messageStream(package);

    /*
     * First take out the message type. The order that you take variables out
     * must match with the order you put variables in. It is also important
     * that all primitive types are from "q" type, such as qint32 instead of int:
     */
    qint8 type;
    messageStream >> type;

    /* Now take out the message: */
    QByteArray message;
    messageStream >> message;

    /* The class that inherits this one will deal with the message: */
    actionMessageReceived(message, user, type);
}

void Room::someoneConnected(User *user)
{
    on_someone_joined_room(user);
}

void Room::someoneDisconnected(User *user)
{
    on_someone_left_room(user);
}

void Room::serverClosed(bool isClosed)
{
    if (!isClosed) {
        qCritical() << "[Room Error] Could not close the server!";
    } else {
        if (roomController->isHost())
            leaveRoom(MSG_NO_MESSAGE);
        else
            leaveRoom(MSG_SERVER_CLOSED);
    }
}

void Room::leftRoom()
{
    //Called when the host says the user left due to some reason.
    leaveRoom(MSG_DROPPED);
}

bool Room::userInRoom()
{
    return isUserInRoom;
}

bool Room::isRoomLoaded()
{
    return roomLoaded;
}

int Room::totalUsersInRoom()
{
    return roomController->getListOfUsers().size();
}

void Room::banned()
{
    leaveRoom(MSG_BANNED);
}

void Room::leaveRoom(ReturnMessages type)
{
    /* Prevent duplicate function calls: */
    if (hasLeftRoom) return;
    hasLeftRoom = true;

    isUserInRoom = false;

    //Called when a signal to leave room is received.
    if (on_leaving_room()) {
        roomController->leaveRoom();
        emit goBackToMenu(type); //The view controller might grab this signal
    } else
        hasLeftRoom = false; //If you could not leave the room, you might try again later.
}

bool Room::log(QListWidgetItem *logItem)
{
    if (roomController == NULL) return false;
    return roomController->addActionLog(logItem);
}

bool Room::logText(QString text, QColor color) {
    QListWidgetItem *item = new QListWidgetItem( QTime::currentTime().toString("[HH:mm:ss] ") + text);
   item->setTextColor(color);
    return log(item);
}

void Room::sendActionMessage(qint8 type, QByteArray message, User *user, bool isTCP)
{
    QByteArray package;

    /* Put the mesage and the type of message inside the package */
    QDataStream messageStream(&package, QIODevice::WriteOnly);
    messageStream << type;
    messageStream << message;

    /* Send message: */
    roomController->sendMessageToServer(RoomController::MSG_ACTION, package, user, isTCP);
}

void Room::sendActionMessageToHost(qint8 type, QByteArray message, bool isTCP)
{
    QByteArray package;

    /* Put the mesage and the type of message inside the package */
    QDataStream messageStream(&package, QIODevice::WriteOnly);
    messageStream << type;
    messageStream << message;

    /* Send message: */
    roomController->sendMessageToServer(RoomController::MSG_ACTION_HOST, package, 0, isTCP);
}

void Room::sendLargeMessage(qint8 type, QByteArray largePackage, QByteArray info, User *user)
{
    QByteArray package;

    /* Put the mesage and the type of message inside the package info */
    QDataStream messageStream(&package, QIODevice::WriteOnly);
    messageStream << type;
    messageStream << info;

    /* Emit a signal to the FileTransfer class asking to send the package: */
    emit sendP(largePackage, package, user);
}

void Room::sendFile(qint8 type, QString filepath, QByteArray info, User *user)
{
    QByteArray package;

    /* Put the mesage and the type of message inside the package info */
    QDataStream messageStream(&package, QIODevice::WriteOnly);
    messageStream << type;
    messageStream << info;

    /* Emit a signal to the FileTransfer class asking to send the package: */
    emit sendF(filepath, package, user);
}

void Room::requestRandomValues(qint32 min, qint32 max, qint16 type, qint32 count, User *to, bool isTCP)
{
    roomController->requestRandomValues(min, max, type, count, to, isTCP);
}

void Room::randomReceived(qint32 min, qint32 max, qint16 type, QList<qint32> values, User *user)
{
   randomValuesReceived(min, max, type, values, user);
}

User* Room::getCurrentUser()
{
    return roomController->getCurrentUser();
}

User *Room::getUser(qint32 userID)
{
    return roomController->searchUser(userID);
}

/** Gets all users in the room: */
QList<User*> Room::getUsersInRoom()
{
    return roomController->getListOfUsers().values();
}

RoomDescription *Room::getRoomDescription()
{
    return roomController->getRoomDescription();
}

void Room::largeMessageStarted(QByteArray info, qint32 size, User *user)
{
    QDataStream messageStream(info);
    qint8 type;
    messageStream >> type;
    QByteArray message;
    messageStream >> message;
    large_message_started(type, message, size, user);
}

void Room::largeMessageProgress(QByteArray info, double percentage, User *user)
{
    QDataStream messageStream(info);
    qint8 type;
    messageStream >> type;
    QByteArray message;
    messageStream >> message;
    large_message_progress(type, message, percentage, user);
}

void Room::largeMessageCancelled(QByteArray info, User *user)
{
    QDataStream messageStream(info);
    qint8 type;
    messageStream >> type;
    QByteArray message;
    messageStream >> message;
    large_message_cancelled(type, message, user);
}

void Room::largeMessageFinished(QByteArray info, QByteArray package, User *user)
{
    QDataStream messageStream(info);
    qint8 type;
    messageStream >> type;
    QByteArray message;
    messageStream >> message;
    large_message_finished(type, message, package, user);
}





