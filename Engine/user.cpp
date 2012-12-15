#include "user.h"

User::User(qint32 id, QObject *parent, UserSettings *settings, bool isUserHost):
    QObject(parent)
{
    /* Make sure the Engine is running: */
    if (!Engine::isRunning()) {
        qCritical() << "[ERROR] Engine class called, but Engine was not started!";
        return;
    }

    if (parent == NULL) {
        qCritical() << "[User] Error: User needs a parent class.";
        return;
    }

    if (settings == NULL) {
        qWarning() << " [User] Error: User needs user settings.";
        return;
    }

    /* Only the room controller can instantiate users: */
    if (!parent->inherits("RoomController")) {
        qCritical() << "[User] Error: You are not allowed to instantiate your own users. This is the Engine's job.";
        return;
    }

    roomController = dynamic_cast<RoomController*> (parent);
    userSettings = settings;
    uniqueID = id;
    creationTime = QTime::currentTime();
    pingTime = QTime::currentTime();
    isYou = false;
    isHost = isUserHost;
    isAdmin = isHost; //Host starts as admin
    isOnline = false;
    latestPing = -1;
    resetNicknameSufix();
}

qint32 User::getUniqueID()
{
    return uniqueID;
}

void User::setUniqueID(qint32 id)
{
    uniqueID = id;
}

QString User::getNickname(bool appendSufix)
{
    if (userSettings == NULL) return "";
    QString name = userSettings->getNickname();
    if ( (appendSufix) && (nicknameSufix > 0) )
        name.append(getNicknameSuffix(nicknameSufix));
    return name;
}

void User::setNickname(QString name)
{
    qDebug() << "SetNickname called";
    if (name != userSettings->getNickname()) {
        userSettings->setNickname(name, OMIT_SIGNAL_DONT_SAVE);
        qDebug() << "resetSufix called";
        resetNicknameSufix();
    }
}

qint32 User::getSecondsSinceCreated()
{
    return creationTime.secsTo(QTime::currentTime());
}

bool User::isThatYou()
{
    return isYou;
}

void User::setIsYou(bool you)
{
    isYou = you;
}

bool User::isTheHost()
{
    return isHost;
}

void User::setIsHost(bool host)
{
    isHost = host;
}

bool User::isAnAdmin()
{
    return isAdmin;
}

void User::setIsAdmin(bool admin)
{
    isAdmin = admin;
}

void User::latencyRequested()
{
    //Start counting the time to know how long it took for the latency to arrive:
    pingTime = QTime::currentTime();
}

qint16 User::getLatency()
{
    latestPing = pingTime.msecsTo(QTime::currentTime());
    return latestPing;
}

QColor User::getMemberChatColor()
{
    return userSettings->getChatColor();
}

void User::setMemberChatColor(QColor color)
{
    userSettings->setChatColor(color, OMIT_SIGNAL_DONT_SAVE);
}

bool User::addChatMessage(QString message, QColor color)
{
    return roomController->addChatMessage(message, color);
}

void User::resetNicknameSufix()
{
    nicknameSufix = 0;
}

bool User::isUserOnline()
{
    return isOnline;
}

void User::setIsOnline(bool online)
{
     isOnline = online;
}

UserSettings* User::getUserSettings()
{
    return userSettings;
}

void User::setUserSettings(UserSettings *settings)
{
    if (!isYou) {
        qWarning() << "[User] You cannot edit the user settings unless this user represents you.";
        return;
    }
    if (settings == userSettings) {
        qWarning() << "[User] Cannot change user settings because they are the same.";
        return;
    }
    delete userSettings;
    userSettings = settings;
    connect( userSettings, SIGNAL(nicknameChanged(QString)), this, SLOT(nicknameChanged(QString)));
    connect( userSettings, SIGNAL(chatColorChanged(QColor)), this, SLOT(chatColorChanged(QColor)));
}

QTime User::getCreationTime()
{
    return creationTime;
}

void User::handleMessage(QByteArray message, User *editor)
{
    /*
      Message received from another member (the editor) who wants
      to do something about this user. Open the message and do
      what has to be done.
      */

    QDataStream messageStream(message);
    qint16 type;
    QString newNickname, oldNickname;
    QColor newColor;
    bool admin;

    messageStream >> type;
    switch (type) {

    case MSG_CHANGE_NICKNAME:
        messageStream >> newNickname;
        oldNickname = getNickname();
        setNickname(newNickname);
        emit nicknameChanged(this, editor, oldNickname);
        break;

    case MSG_CHANGE_CHAT_COLOR:
        messageStream >> newColor;
        setMemberChatColor(newColor);
        emit chatColorChanged(this);
        break;

    case MSG_CHANGE_ADMIN_STATUS:
        messageStream >> admin;
        setIsAdmin(admin);
        emit adminStatusChanged(this, editor);
        break;

    default :
        //Delegates to the class that extends User:
        onMessageReceived(messageStream, type, editor);
    }
}

QByteArray User::prepareMessage(qint16 type)
{
    QByteArray message;
    QDataStream messageStream(&message, QIODevice::WriteOnly);
    messageStream << type;

    switch (type) {

    case MSG_CHANGE_NICKNAME:
        messageStream << getNickname(false);
        break;

    case MSG_CHANGE_CHAT_COLOR:
        messageStream << getMemberChatColor();
        break;

    case MSG_CHANGE_ADMIN_STATUS:
        messageStream << isAnAdmin();
        break;

    default :
        onMessageRequest(messageStream, type);
    }

    QByteArray package;
    QDataStream packageStream(&package, QIODevice::WriteOnly);
    packageStream << getUniqueID(); //the id of who is being edited
    packageStream << message;
    return package;
}

QByteArray User::getDataPackage()
{
    QByteArray message;
    QDataStream messageStream(&message, QIODevice::WriteOnly);
    messageStream << userSettings;
    messageStream << uniqueID;
    messageStream << nicknameSufix;
    messageStream << isAdmin;
    addDataToStream(messageStream);
    return message;
}

void User::setDataPackage(QByteArray message)
{
    QDataStream messageStream(message);
    messageStream >> userSettings;
    messageStream >> uniqueID;
    messageStream >> nicknameSufix;
    messageStream >> isAdmin;
    useDataFromStream(messageStream);
    emit dataUpdated();
}

bool User::isEqual(User *user)
{
    return ((user != NULL) && (*user == *this));
}

/**
  MakeNameUnique is the function responsible for putting the nickname sufix in the username.
  The nickname sufix is the number in the end of the username when there are multiple users
  with the same nickname.
  @param allMembers: The list of all users in the room to be able to compare nicknames.
  */
void User::makeNameUnique(QList<User*> allMembers)
{
    QString name = getNickname();

    /* No need to change name if there's no conflict: */
    if (!nameConflict(name, allMembers))
        return;

    qint8 repetitions = 1;
    resetNicknameSufix();
    QString newName(name);
    bool isThereNameConflict;
    do {
        repetitions++;
        newName = name + getNicknameSuffix(repetitions);
        isThereNameConflict = nameConflict(newName, allMembers);
    } while (isThereNameConflict);
    nicknameSufix = repetitions;
}

QString User::getNicknameSuffix(qint8 number)
{
    return QString(" (") + QString::number(number) + ")";
}

/** @return true if any user in @param allMembers have the @param name */
bool User::nameConflict(QString name, QList<User *> allMembers)
{
    foreach (User *user, allMembers) {
        if ( (this != user) && (name == user->getNickname()) ) {
            return true;
        }
    }
    return false;
}

void User::chatColorChanged(QColor)
{
    sendEditUserMessage(MSG_CHANGE_CHAT_COLOR);
}

void User::nicknameChanged(QString name)
{
    setNickname(name);
    sendEditUserMessage(MSG_CHANGE_NICKNAME);
}

void User::sendEditUserMessage(qint16 type)
{
    if (!isOnline)
        qWarning() << "[User] Cannot send a message when the user is offline.";
    else
        roomController->sendMessageToServer( RoomController::MSG_EDIT_ROOM_MEMBER, prepareMessage(type) );
}

bool User::hasPermission(qint8 permissionIndex, bool isOwnerOfObject)
{
    bool permission = false;

    switch (permissionIndex) {

    case RoomDescription::PERMISSION_EVERYONE:
        permission = true;
        break;

    case RoomDescription::PERMISSION_ADMINS_ONLY:
        permission = isAnAdmin();
        break;

    case RoomDescription::PERMISSION_NOBODY:
        permission = false;
        break;

    case RoomDescription::PERMISSION_HOST_ONLY:
        permission = isTheHost();
        break;

    case RoomDescription::PERMISSION_YOURS_ONLY:
        permission = isOwnerOfObject;
        break;

    default :
        qWarning() << "[UserDetails] ERROR: Permission " << permissionIndex << " is invalid!";

    }

    return permission;
}

bool User::operator ==(const User& other) const
{
    if (&other == NULL) return false;
    return ( (uniqueID == other.uniqueID) && (creationTime == other.creationTime) );
}

bool User::operator !=(const User& other) const
{
    return !(*this == other);
}


