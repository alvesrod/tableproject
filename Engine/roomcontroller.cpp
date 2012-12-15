#include "roomcontroller.h"

RoomController::RoomController(RoomDescription *description, UserSettings *settings)
{
    if (description == NULL) {
        qWarning() << "Cannot instantiate a Room Controller without a Room Description.";
        return;
    }

    /* Make sure the Engine is running: */
    if (!Engine::isRunning()) {
        qCritical() << "[ERROR] Engine class called, but Engine was not started!";
        return;
    }

    totalOfGenericMessages = 0;
    currentUserID = -1;
    hasLeftRoom = false;
    currentUser = NULL; //You don't know who you are until the host tells you.
    hostUser = NULL;
    chatroom = NULL;
    userSettings = settings;
    roomDescription = description;
    roomDescription->setParent(this);

    /* Prepare and start the Network: */
    networkThread = new Network(this);
    networkThread->start();
    setupNetworkConnections();

    if (DISPLAY_RC_DEBUG_MESSAGES)
        qDebug() << "[RoomSettings] A new Room has been declared.";
}

RoomController::~RoomController()
{
    leaveRoom();

    /* Delete the network thread when it is finished: */
    connect(networkThread, SIGNAL(finished()), networkThread, SLOT(deleteLater()));

    networkThread->exit();
    if (DISPLAY_RC_DEBUG_MESSAGES)
        qDebug() << "Room Controller deleted.";
}

/** ===== TWO MAIN NETWORK METHODS (SEND AND RECEIVE): ===== **/

/**
This function receives a message from the network.
It reads the message to decide what to do with it and
which signals to emit:
*/
void RoomController::networkMessageReceived(qint32 sender, QByteArray package)
{
    //Prepare message to be read:
    QByteArray message;
    qint8 type;
    QDataStream packageStream(package);
    User *senderProfile = NULL;

    packageStream >> type; //Take the type of message out of the package.
    packageStream >> message; //Take the message out of the package.

    if (DISPLAY_RC_DEBUG_MESSAGES)
        qDebug() << "[RoomController] Received message type" << type << "from" << sender;

    /*
     * Move on without a sender only if message is from host or from type(s):
     * MSG_PREPARE_ROOM_REPLY, MSG_PREPARE_ROOM, MSG_REQUEST_ROOM_INFO,
     * MSG_ROOM_PREPARED, MSG_REMOVE_USER, MSG_REQUEST_ROOM_INFO_REPLY
     * MSG_ACTION, MSG_ACTION_HOST, MSG_OTHER.
     */
    if (!getSenderAndCheckMessage(sender, senderProfile, type)) return;

    //Read message:
    switch (type) {

    case MSG_PREPARE_ROOM:
        prepareRoomReadMessage(senderProfile, message, sender);
        break;

    case MSG_PREPARE_ROOM_REPLY:
        prepareRoomReplyReadMessage(sender, message);
        break;

    case MSG_ROOM_PREPARED:
        roomPreparedMessage(message, sender);
        break;

    case MSG_BAN_USER:
        leaveRoom();
        emit banned();
        break;

    case MSG_REMOVE_USER:
        leaveRoom();
        emit leftRoom();
        break;

    case MSG_REQUEST_ROOM_INFO:
        prepareRequestRoomInfoMessage(sender);
        break;

    case MSG_REQUEST_ROOM_INFO_REPLY:
        roomDescription->setDataPackage(message);
        emit roomHasInfoReady();
        break;

    case MSG_PING:
        preparePingMessage(message);
        break;

    case MSG_PING_REPLY:
        preparePingMessageReply(message);
        break;

    case MSG_CHAT_MESSAGE:
        emit chatMessageReceived(message, senderProfile);
        break;

    case MSG_PRIVATE_CHAT_MESSAGE:
        emit privateChatMessageReceived(privateMessageCrypt(message, sender, currentUserID), senderProfile);
        break;

    case MSG_EDIT_ROOM_MEMBER:
        prepareUserMessage(message, senderProfile);
        break;

    case MSG_ACTION:
    case MSG_ACTION_HOST:
        emit actionMessageReceived(message, senderProfile);
        break;

    case MSG_RANDOM_VALUE:
        prepareRandomValueMessage(message, senderProfile);
        break;

    case MSG_RANDOM_VALUE_REPLY:
         prepareRandomValueMessageReply(message, senderProfile);
        break;

    case MSG_DOWNLOAD:
        emit downloadMessageReceived(message, senderProfile);
        break;

    case MSG_OTHER:
    default :
        dealWithGenericMessageReceived(message, senderProfile);
    }
}

/**
This function sends a message to the network.
It writes the message based on its type:
*/
void RoomController::sendMessageToServerByID(RoomController::NetworkMessageTypes type, QByteArray message, qint32 msgTo, bool isTCP)
{
    //Prepare message to be sent:
    QByteArray package;
    QDataStream packageStream(&package, QIODevice::WriteOnly); //Opens a package to be read
    packageStream << (qint8) type; //Add the type of the message into the package.

    if (DISPLAY_RC_DEBUG_MESSAGES)
        qDebug() << "Sent message type" << type << "to" << msgTo;

    switch (type) {
    case MSG_PREPARE_ROOM:
        packageStream << getPrepareRoomMessage();
        if (makeSureUserSettingsIsDeclared())
            sendPackage(DESCRIPTOR_CODE_FOR_HOST, package, TCP_MESSAGE);
        break;

    case MSG_ROOM_PREPARED:
        packageStream << getRoomPreparedMessage();
        sendPackage(BROADCAST_DESCRIPTOR, package, TCP_MESSAGE); //send to all because everyone wants the user's nickname.
        break;

    case MSG_REQUEST_ROOM_INFO:
        packageStream << message;
        sendPackage(DESCRIPTOR_CODE_FOR_HOST, package, TCP_MESSAGE);
        break;

    case MSG_REQUEST_ROOM_INFO_REPLY:
        roomDescription->setTotalUsersInRoom( users.size() );
        roomDescription->setHostNicknamePrediction( currentUser->getNickname() );
        packageStream << roomDescription->getBasicDataPackage();
        sendPackage(msgTo, package, isTCP);
        break;

    case MSG_PING:
        packageStream << getPreparePingMessage();
        if (msgTo == BROADCAST_DESCRIPTOR) return;
        sendPackage(msgTo, package, UDP_MESSAGE);
        break;

    case MSG_BAN_USER:
        addUserToBannedList(msgTo);
        packageStream << message;
        sendPackage(msgTo, package, TCP_MESSAGE);
        break;

    case MSG_RANDOM_VALUE:
        packageStream << message;
        sendPackage(DESCRIPTOR_CODE_FOR_HOST, package, isTCP);
        break;

    case MSG_ACTION_HOST:
        packageStream << message;
        sendPackage(DESCRIPTOR_CODE_FOR_HOST, package, isTCP);
        break;

    case MSG_PRIVATE_CHAT_MESSAGE:
        packageStream << privateMessageCrypt(message, currentUserID, msgTo);
        sendPackage(msgTo, package, isTCP);
        break;

    default :
        packageStream << message;
        sendPackage(msgTo, package, isTCP);
    }
}

void RoomController::sendMessageToServer(NetworkMessageTypes type, QByteArray message, User *to, bool isTCP)
{
    qint32 msgTo = (to == TO_EVERYONE)? BROADCAST_DESCRIPTOR : to->getUniqueID();
    sendMessageToServerByID(type, message, msgTo, isTCP);
}


/** GETTERS AND SETTERS: **/

QHash<qint32, User*> RoomController::getListOfUsers() { return users; }

QList<User *> RoomController::getPrimitiveListOfUsers() {
    return users.values();
}

User *RoomController::getCurrentUser() { return currentUser; }

User *RoomController::getHostUser() { return hostUser; }

RoomDescription* RoomController::getRoomDescription() { return roomDescription; }

bool RoomController::isHost() { return roomDescription->isHostRoom(); }

UserSettings *RoomController::getUserSettings() { return userSettings; }

void RoomController::setPassword(QString password) { roomDescription->setPassword(password); }
QString RoomController::getPassword() { return roomDescription->getPassword(); }

void RoomController::setChatroom(ChatRoom *chat) { chatroom = chat; }
ChatRoom *RoomController::getChatroom() { return chatroom; }


/** ===== PUBLIC NETWORK METHODS: ===== **/
void RoomController::startNewServer()
{
    generatedKey = generateRoomCryptKey(); //Get a random key for the room.
    emit networkStartNewServer( roomDescription->getRoomPort() );
}

void RoomController::closeServer()
{
    /* Only the host can close the server: */
    if (!isHost()) return;

    /* Don't remove the published room if the server as closed
      before joining the room. The room will only be published
      AFTER you joined the room. */
    if (currentUser != NULL) {        
        /* Ask the WebContact to remove this room from the public list: */
        connect(this, SIGNAL(removePublishedRoom(qint32)), WebContact::getInstance(), SLOT(removePublishedRoom(qint32)), Qt::QueuedConnection);
        emit removePublishedRoom( roomDescription->getRoomPort() );
    }

    /* Signal the network to close the port: */
    emit networkCloseServer();
}

void RoomController::disconnectFromServer()
{
    emit networkDisconnectFromServer();
}

void RoomController::leaveRoom()
{
    if (hasLeftRoom) return; //Make sure the user didn't leave the room already
    hasLeftRoom = true;
    disconnectFromServer();
    if (isHost())
        closeServer();
}

void RoomController::sendDownloadMessageToServer(QByteArray message, User *to)
{
    sendMessageToServer(MSG_DOWNLOAD, message, to, true);
}

void RoomController::connectToServer() {
    hasLeftRoom = false;
    emit networkConnectToServer( roomDescription->getRoomIp() , roomDescription->getRoomPort() );
}

/** ===== PRIVATE NETWORK SLOTS: ===== **/
void RoomController::networkErrorMessage(QString error) { emit errorMessage(error); }
void RoomController::networkDisconnectedFromServer(bool success) { emit disconnectedFromServer(success); }
void RoomController::networkServerStarted(bool started, quint16 port)
{
    getRoomDescription()->setRoomPort(port);
    emit serverStarted(started);
}

void RoomController::networkConnectedToServer(bool connected) { emit connectedToServer(connected); }
void RoomController::networkMessageSent(bool sent) { emit messageSent(sent); }
void RoomController::networkServerClosed(bool closed) { emit serverClosed(closed); }

void RoomController::someoneDisconnected(qint32 descriptor)
{
    //If the user was in the loading screen, removes it from the "comingUsers" array:
    removeUserFromComingUsersArray(descriptor);

    User *user = searchUser(descriptor);
    if (user == NULL) {
        if (isHost())
            if (!isInPingUsersList(descriptor)) //Maybe the user was just cheking out the room
                addUserToLatestUnknownDisconnects(descriptor);
    } else {
        if (user == currentUser) {
            qDebug() << "You are leaving the Room.";
        }
        user->setIsOnline(false);
        emit someoneDisconnectedFromServer(user);
        users.remove(user->getUniqueID());
        roomDescription->setTotalUsersInRoom( users.size() );
    }
}

/**
    Connect network signal and slots:
    QueuedConnections are important because the network is in a different thread.
**/
void RoomController::setupNetworkConnections()
{
    connect(this, SIGNAL(networkStartNewServer(qint32)), networkThread, SLOT(startNewServer(qint32)), Qt::QueuedConnection);
    connect(this, SIGNAL(networkConnectToServer(QString,qint32)), networkThread, SLOT(connectToServer(QString,qint32)), Qt::QueuedConnection);
    connect(this, SIGNAL(networkSendMessageToServer(QByteArray,qint32, bool)), networkThread, SLOT(sendMessageToServer(QByteArray, qint32, bool)), Qt::QueuedConnection);
    connect(this, SIGNAL(networkCloseServer()), networkThread, SLOT(closeServer()), Qt::QueuedConnection);
    connect(this, SIGNAL(networkDisconnectFromServer()), networkThread, SLOT(disconnectFromServer()), Qt::QueuedConnection);
    connect(networkThread, SIGNAL(networkMessage(qint32, QByteArray)), this, SLOT(networkMessageReceived(qint32, QByteArray)), Qt::QueuedConnection);
    connect(networkThread, SIGNAL(errorMessage(QString)), this, SLOT(networkErrorMessage(QString)), Qt::QueuedConnection);
    connect(networkThread, SIGNAL(disconnectedFromServer(bool)), this, SLOT(networkDisconnectedFromServer(bool)), Qt::QueuedConnection);
    connect(networkThread, SIGNAL(serverStarted(bool,quint16)), this, SLOT(networkServerStarted(bool,quint16)), Qt::QueuedConnection);
    connect(networkThread, SIGNAL(connectedToServer(bool)), this, SLOT(networkConnectedToServer(bool)), Qt::QueuedConnection);
    connect(networkThread, SIGNAL(messageSent(bool)), this, SLOT(networkMessageSent(bool)), Qt::QueuedConnection);
    connect(networkThread, SIGNAL(serverClosed(bool)), this, SLOT(networkServerClosed(bool)), Qt::QueuedConnection);
    connect(networkThread, SIGNAL(someoneDisconnected(qint32)), this, SLOT(someoneDisconnected(qint32)), Qt::QueuedConnection);
}

User* RoomController::searchUser(qint32 id) //Return NULL if not found
{
    return users.value(id, NULL);
}

bool RoomController::addUser(User *user)
{
    if (user == NULL) {
        qCritical() << "[ERROR] Trying to add a null user.";
        return false;
    }

    //If the user was in the loading screen, removes it from the "comingUsers" array:
    removeUserFromComingUsersArray(user->getUniqueID());

    User* other = searchUser(user->getUniqueID());
    if (other != NULL) {
            qWarning() << "User (" << user->getUniqueID() << ") already found in database.";
            if (user != other) user->deleteLater(); //delete only if they don't share the same pointer.
            return false;
    }

    user->setIsOnline(true);
    user->makeNameUnique( getPrimitiveListOfUsers() );
    if (DISPLAY_RC_DEBUG_MESSAGES)
        qDebug() << "User" << user->getUniqueID() << "added to the Room.";
    users[user->getUniqueID()] = user;
    roomDescription->setTotalUsersInRoom( users.size() );
    return true;
}

bool RoomController::isRoomFull()
{
    return ( (users.size() + comingUsers.size()) >= roomDescription->getMaxNumberOfUsers());
}

QByteArray RoomController::getPrepareRoomMessage()
{
    QByteArray message;
    QDataStream messageStream(&message, QIODevice::WriteOnly);
    messageStream << Engine::getAppVersion();
    messageStream << Engine::getEngineVersion();
    messageStream << roomDescription->getPassword();

    QString ip;
    if (isHost())
        ip = QString(LOCAL_IP); //Host has a local ip.
    else
        ip = userSettings->getIp();

    messageStream << ip;
    messageStream << getSecretApplicationKey(ip);
    return message;
}

QByteArray RoomController::getRoomPreparedMessage()
{
    QByteArray message;
    QDataStream messageStream(&message, QIODevice::WriteOnly);
    messageStream << userSettings;
    return message;
}

QByteArray RoomController::getPreparePingMessage()
{
    QByteArray message;
    QDataStream messageStream(&message, QIODevice::WriteOnly);
    messageStream << currentUserID;
    return message;
}

void RoomController::prepareRequestRoomInfoMessage(qint32 id)
{
    addToPingUsersList(id);
    sendMessageToServerByID(MSG_REQUEST_ROOM_INFO_REPLY, 0, id);
}


void RoomController::sendPackage( qint32 to, QByteArray package, bool TCP )
{
    emit networkSendMessageToServer(package, to, TCP);
}

void RoomController::prepareRoomReplyReadMessage( qint32 sender, QByteArray message )
{
	//Break the reply and get all the information about the Room:
	qint8 answer;
    QString version, engineVersion;
	QDataStream messageStream(message);
	messageStream >> answer;

	switch (answer) {

	case ANSWER_ROOM_FULL:
        emit roomPrepared(false, tr("The room is full."));
		break;

    case ANSWER_DISCONNECTED_RECENTLY:
    case ANSWER_DENIED_OTHER:
        emit roomPrepared(false, tr("The host refused your connection for some reason. Please, try again in ")
                              + QString::number(TIMEOUT_WAITING_DISCONNECT_SECONDS) + tr(" seconds."));
        break;

	case ANSWER_WRONG_VERSION:
		messageStream >> version;
        messageStream >> engineVersion;
        emit roomPrepared(false, tr("Wrong version. Your version is") + " " + Engine::getAppVersion()
                          + " (" + tr("engine") + " " + Engine::getEngineVersion() + ") "
                          + tr("and this room is using version") + " " + version + " ("
                          + tr("engine") + " " + Engine::getEngineVersion() + "). ");
		break;

    case ANSWER_WRONG_APP_KEY:
        emit roomPrepared(false, tr("The host application is different from yours."));
        break;

    case ANSWER_WRONG_PASSWORD:
        emit roomPrepared(false, tr("Wrong password."));
        break;

	case ANSWER_ALLOWED:
        prepareRoom(sender, messageStream);
		break;

    case ANSWER_BANNED:
        emit roomPrepared(false, tr("The host did not allow you to join this room."));
        break;

	default :
		qDebug() << "Received an invalid answer: " << answer;
    }
}

void RoomController::prepareUserMessage(QByteArray message, User* senderProfile)
{
    QDataStream messageStream(message);
    qint32 memberID;
    QByteArray memberMessage;
    messageStream >> memberID;
    messageStream >> memberMessage;
    User *memberToEdit = searchUser(memberID);
    if (memberToEdit == NULL) {
        qWarning() << "Tried to edit the profile of a member" << memberID << "who is not in the room.";
        return;
    }
    memberToEdit->handleMessage(memberMessage, senderProfile);
}

void RoomController::prepareRandomValueMessage(QByteArray message, User *senderProfile)
{
    if (!isHost()) {
        qDebug() << "[RoomController] You received a message that only the host should have received.";
        return;
    }
    QDataStream messageStream(message);
    qint32 min, max, random, to, count;
    qint16 type;
    messageStream >> min;
    messageStream >> max;
    messageStream >> type;
    messageStream >> count;
    messageStream >> to;

    if ( (min >= max) || (count < 1) || (count > MAX_RANDOM_VALUES_COUNT) )  {
        qCritical() << "[RoomController error] Received an invalid random value [" << min << "," << max << "|" << count << "].";
        qDebug() << "[RoomController] Max num of random values allowed:" << MAX_RANDOM_VALUES_COUNT;
        return;
    }

    //Build the reply:
    QByteArray reply;
    QDataStream replyStream(&reply, QIODevice::WriteOnly);
    replyStream << min;
    replyStream << max;
    replyStream << type;
    replyStream << count;

    QList<qint32> values;

    for (qint32 i = 0; i < count; i++) {
        random = ( qrand() % (max-min+1) ) + min;
        values << random;
    }

    replyStream << values;
    replyStream << senderProfile->getUniqueID();

    //Send reply:
    sendMessageToServerByID(MSG_RANDOM_VALUE_REPLY, reply, to);
}

void RoomController::prepareRandomValueMessageReply(QByteArray message, User *senderProfile)
{
    if (!senderProfile->isTheHost()) {
        qDebug() << "[RoomController error] Received random value, but not from the host. Message ignored.";
        return;
    }

    QDataStream messageStream(message);
    qint32 min, max, from, count;
    qint16 type;
    messageStream >> min;
    messageStream >> max;
    messageStream >> type;
    messageStream >> count;

    if (count > MAX_RANDOM_VALUES_COUNT) {
        qWarning() << "[RoomController error] Received more random values than allowed. The limit is " << MAX_RANDOM_VALUES_COUNT;
        return;
    }

    if (min >= max)  {
        qWarning() << "[RoomController error] Received an invalid boundary [" << min << ", " << max << "].";
        return;
    }

    QList<qint32> values;
    messageStream >> values;

    if (values.size() != count) {
        qWarning() << "[RoomController error] Received " << values.size() << "values. Expecting: " << count;
        return;
    }

    messageStream >> from;

    senderProfile = searchUser(from);

    if (senderProfile == NULL) {
        qWarning() << "[RoomController error] Received a random value from a user who is not in the room. Message ignored";
        return;
    }

    emit randomValuesReceived(min, max, type, values, senderProfile);
}

void RoomController::prepareRoomReadMessage( User* senderProfile, QByteArray message, qint32 sender )
{
    if (senderProfile != NULL) {
        qDebug() << "[RoomController error] Cannot prepare room for someone who already have a room prepared.";
        return;
    }

    if (!isHost()) {
        qDebug() << "[RoomController] You received a message that only the host should have received.";
        return;
    }

    QDataStream messageStream(message);

    //First, search the sender ID in the list of latestUnknownDisconnects.
    //This is because maybe the user already disconnected, but due to a slow
    //connection, the others are only receiving this message now!
    bool recentlyDisconnected = wasUserRecentlyDisconnected(sender);

    //Get the user version:
    QString userAppVersion, userEngineVersion;
    messageStream >> userAppVersion;
    messageStream >> userEngineVersion;

    //Get the typed password:
    QString password;
    messageStream >> password;

    //Get the user ip:
    QString ip;
    messageStream >> ip;

    //Get the user secret key:
    QByteArray key;
    messageStream >> key;

    //Build the reply:
    QByteArray reply;
    QDataStream replyStream(&reply, QIODevice::WriteOnly);
    if (isRoomFull())
        replyStream << (qint8) ANSWER_ROOM_FULL;
    else {
        if ( (userAppVersion != Engine::getAppVersion()) || (userEngineVersion != Engine::getEngineVersion()) ) {
            qDebug() << "User joined with the wrong version: " << userAppVersion << ". E. " << userEngineVersion;
            replyStream << (qint8) ANSWER_WRONG_VERSION;
            replyStream << Engine::getAppVersion();
            replyStream << Engine::getEngineVersion();
        } else {
            if (bannedUsers.contains(ip)) {
                /* User ip is in the blacklist. So, don't let him join: */
                replyStream << (qint8) ANSWER_BANNED;
            } else {
                if (recentlyDisconnected) {
                    replyStream << (qint8) ANSWER_DISCONNECTED_RECENTLY;
                } else {
                    if (password != roomDescription->getPassword()) {
                        replyStream << (qint8) ANSWER_WRONG_PASSWORD;
                    } else {
                        if (key != getSecretApplicationKey(ip))
                            replyStream << (qint8) ANSWER_WRONG_APP_KEY;
                        else {
                            /* Make sure the first user that joins the room is the host: */
                            if ( (currentUser == NULL) && (ip != roomDescription->getRoomIp()) ) {
                                replyStream << (qint8) ANSWER_DENIED_OTHER;
                            } else {
                                /* Alright, let the user come in: */
                                comingUsers << sender; //since you are allowing this player to join, make sure nobody grabs his spot.
                                replyStream << (qint8) ANSWER_ALLOWED;
                                replyStream << (qint32) sender; //send the ID, so the user can know its own id.
                                replyStream << crypt(generatedKey, "187425446", false);

                                roomDescription->setTotalUsersInRoom( users.size() );
                                if (currentUser)
                                    roomDescription->setHostNicknamePrediction( currentUser->getNickname() );
                                replyStream << roomDescription;
                                foreach (User *user, users) {
                                    replyStream << user;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    //Send reply:
    sendMessageToServerByID(MSG_PREPARE_ROOM_REPLY, reply, sender);
}

void RoomController::prepareRoom( qint32 sender, QDataStream &messageStream )
{
    if (DISPLAY_RC_DEBUG_MESSAGES)
        qDebug() << "User " << sender << " is allowed to join the room.";

    if (isHost()) {
        //Host found himself:
        currentUserID = sender;
        roomDescription->setHostNicknamePrediction( userSettings->getNickname() );
        emit roomPrepared(true);
        return;
    }

    qint32 id;
    messageStream >> id; //get your id.
    messageStream >> generatedKey; //room filetransfer key.
    generatedKey = crypt(generatedKey, "187425446", false);
    messageStream >> roomDescription;
    qint16 totalUsers = roomDescription->getTotalUsersInRoom();

    for (qint16 i = 0; i < totalUsers; i++) {
        User *newMember = Engine::newUser(INVALID_ID, this, Engine::newUserSettings(this,true));
        messageStream >> newMember;

        if (sender == newMember->getUniqueID()) { //The host sent the message, so compare with the sender.
            newMember->setIsHost(true);
            if (addUser(newMember))
                hostUser = newMember;
        } else addUser(newMember);
    }

    User *user = searchUser(id);
    if (user != NULL) {
        qWarning() << "[RoomController] Warning: you are already in this room! You shouldn't be";
    } else {
        currentUserID = id;
        emit roomPrepared(true);
    }
}

void RoomController::preparePingMessage(QByteArray message)
{
    qint32 from;
    QDataStream messageStream(message);
    messageStream >> from;
    QByteArray reply;
    QDataStream replyStream(&reply, QIODevice::WriteOnly);
    replyStream << currentUserID;

    sendMessageToServerByID(MSG_PING_REPLY, reply, from, UDP_MESSAGE);
}

void RoomController::preparePingMessageReply(QByteArray message)
{
    qint32 from;
    QDataStream messageStream(message);
    messageStream >> from;

    User *user = searchUser(from);
    if (user == NULL) {
        qDebug() << "[RoomController] Received a ping from user" << from << "who is not in the room. Ping ignored.";
        return;
    }
    emit pingReceived(user);
}

bool RoomController::getSenderAndCheckMessage( qint32 sender, User* &senderProfile, qint8 type )
{
    if (sender == DESCRIPTOR_CODE_FOR_HOST) //message came from host (no sender).
        return true;
    else
        senderProfile = searchUser(sender);

	if (senderProfile == NULL) {
		switch (type) {
            case MSG_PREPARE_ROOM:
			case MSG_PREPARE_ROOM_REPLY:
            case MSG_ROOM_PREPARED:
            case MSG_REQUEST_ROOM_INFO:
            case MSG_REQUEST_ROOM_INFO_REPLY:
            case MSG_REMOVE_USER:
            case MSG_OTHER:
            case MSG_ACTION:
            case MSG_ACTION_HOST:
                break; //Those types of message do not need a user profile.
			default:
                qWarning() << "[RoomController] Received a message type" << type << "from a sender (" << sender << ") who is not in the Room.";
				return false;
		}
	}
	return true;
}

void RoomController::roomPreparedMessage(QByteArray message, qint32 sender)
{
    QDataStream messageStream(message);
    UserSettings *uSettings = Engine::newUserSettings(this);
    messageStream >> uSettings; //get the user nickname
    User *senderProfile = Engine::newUser(sender, this, uSettings);
    if (sender == currentUserID) { //you found yourself!
        currentUser = senderProfile;
        uSettings->deleteLater();
        senderProfile->setIsYou(true);
        senderProfile->setUserSettings(userSettings);
        if (isHost()) {
            senderProfile->setIsHost(true);
            hostUser = senderProfile;
            senderProfile->setIsAdmin(true); //Host starts as an admin
        }
    }
    if ( addUser(senderProfile) )
        emit someoneConnectedToServer(senderProfile);

    if ((isHost()) && (wasUserRecentlyDisconnected(sender))) {
        qDebug() << "[RoomController] User" << sender << "was recently disconnected. Request to join room denied.";
        sendMessageToServerByID(MSG_REMOVE_USER, 0, sender);
    }
}

bool RoomController::wasUserRecentlyDisconnected(qint32 userID)
{
    //This is because maybe the user already disconnected, but due to a slow
    //connection, the others are only receiving this message now!

    foreach (User *user, latestUnknownDisconnects) {
        if ( (user->getUniqueID() == userID) && (user->getSecondsSinceCreated() < TIMEOUT_WAITING_DISCONNECT_SECONDS) ) {
            qDebug() << "User" << userID << "disconnected only" << user->getSecondsSinceCreated() << "seconds ago.";
            return true;
        }
    }
    return false;
}

void RoomController::removeUserFromComingUsersArray(qint32 id)
{
    comingUsers.removeAll(id);
}

void RoomController::dealWithGenericMessageReceived(QByteArray message, User *user)
{
    totalOfGenericMessages++;
    qDebug() << "[RoomController] Total of generic messages received: " << totalOfGenericMessages;
    emit genericMessageReceived(message, user);
}

void RoomController::addToPingUsersList(qint32 id)
{
    if (isInPingUsersList(id)) return;
    if (pingUsers.size() > MAX_NUMBER_OF_PING_USERS) {
        pingUsers.removeAt(0);
    }
    pingUsers << id;
}

bool RoomController::isInPingUsersList(qint32 id)
{
    return (pingUsers.indexOf(id) >= 0);
}

void RoomController::addUserToLatestUnknownDisconnects(qint32 id)
{
    qDebug() << "User (" << id << ") left, but this user was not on the Room (added to latest disconnects).";
    //Add the unknown user to a list of disconnected users in case this user shows up.
    if (latestUnknownDisconnects.size() >= MAX_NUMBER_OF_DISCONNECTS) {
        User *user = latestUnknownDisconnects.at(0);
        latestUnknownDisconnects.removeAt(0);
        delete user;
    }
    latestUnknownDisconnects << Engine::newUser(id, this, Engine::newUserSettings(this,true));
}

bool RoomController::makeSureUserSettingsIsDeclared()
{
    if (userSettings == NULL) {
        qCritical() << "[Room Controller] ERROR: Room Controller requires some user setting to be able to continue.";
        return false;
    }
    return true;
}

void RoomController::addUserToBannedList(qint32 id)
{
    if (!isHost()) {
        qDebug() << "[RoomController] Error: Only the host can ban users";
        return;
    }

    User *user = searchUser(id);
    if (user == NULL) {
        qDebug() << "[RoomController] Tried to ban an user who is not in the room.";
        return;
    }

    /* Add the user to the blacklist, so he/she can't join the room again: */
    bannedUsers[ user->getUserSettings()->getIp() ] = user;

    qDebug() << user->getNickname() << " was banned (IP: " << user->getUserSettings()->getIp() << ").";
}

/**
 * This is a key that will be generated and sent to the users when they
 * join the room. This key can then be used for cryptographed messages
 * such as the transference of files.
 */
QByteArray RoomController::generateRoomCryptKey()
{
    QString secretKey = QString(getRoomDescription()->getRoomPort())
            + Engine::getAppSecret() + QString::number( rand() * 1000 );
    return QCryptographicHash::hash((secretKey.toAscii()),QCryptographicHash::Md5);
}

/**
 * The secret key is sent through the loading screen
 * to make sure the application name matches between
 * client and server. Their keys must match for them
 * to trade network messages.
 */
QByteArray RoomController::getSecretApplicationKey(QString userIP)
{    
    QString secretKey = QString(getRoomDescription()->getRoomPort())
            + Engine::getAppCompany() + Engine::getAppName()
            + roomDescription->getPassword() + userIP + Engine::getAppSecret();
    return QCryptographicHash::hash((secretKey.toAscii()),QCryptographicHash::Md5);

    /*
     * This key could be a lot more elaborated by hashing the application data. But,
     * since the application is cross-platform, we can't hash the file because the
     * Mac file and Windows file are different (and each user just have one or the other).
     */
}

/** Crypt the private messages: */
QByteArray RoomController::privateMessageCrypt(QByteArray message, qint32 id1, qint32 id2)
{
    QByteArray key("pchat");
    key.append(id1);
    key.append(id2);
    return crypt(message, key);
}


bool RoomController::addChatMessage(QString message, QColor color)
{
    if (chatroom == NULL) return false;
    chatroom->addChatMessage(message, 0, color);
    return true;
}

bool RoomController::addActionLog(QListWidgetItem *logItem)
{
    if (chatroom == NULL) return false;
    chatroom->addLog(logItem);
    return true;
}

void RoomController::requestRandomValues(qint32 min, qint32 max, qint16 type, qint32 count, User *to, bool isTCP)
{
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream << min;
    stream << max;
    stream << type;
    stream << count;

    /* Get the id of who the message is to. If the message is to everyone, the id is 0: */
    qint32 id = (to == TO_EVERYONE) ? TO_EVERYONE : to->getUniqueID();
    stream << id;

    if ( (isHost()) && (currentUser != NULL) )
        /* No need to send a message to the host if the current user is the host: */
        prepareRandomValueMessage(message, currentUser);
    else
        /* Send to the host a message asking for the random value: */
        sendMessageToServerByID(MSG_RANDOM_VALUE, message, DESCRIPTOR_CODE_FOR_HOST, isTCP);
}

QByteArray RoomController::crypt(QByteArray package, QByteArray localKey, bool useHostKey)
{
    if(localKey.isEmpty())
      return package;

    /* Combine keys: */
    QByteArray key;
    if (useHostKey) key.append(generatedKey); //Key received from the host
    key.append(localKey); //Key stored locally.
    key = QCryptographicHash::hash(key,QCryptographicHash::Md5);

    QByteArray crypt;
    for(int i = 0 , j = 0; i < package.length(); ++i , ++j)
    {
      if (j == key.length()) j = 0;
      crypt.append(package.at(i) ^ key.at(j));
    }
    return crypt;
}
