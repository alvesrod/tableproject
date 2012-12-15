#include "roomdescription.h"

RoomDescription::RoomDescription(qint32 port, QString ip, QObject *parent) :
    QObject(parent)
{
    /* Make sure the Engine is running: */
    if (!Engine::isRunning()) {
        qCritical() << "[ERROR] Engine class called, but Engine was not started!";
        return;
    }

    isHost = false;
    isPasswordProtected = false;
    isToShowLocation = false;
    isPublished = false;
    password = "";
    roomPort = port;
    roomIP = ip;
    maxNumberOfUsers = -1;
    totalUsersInRoom = -1;
    roomType = "[NO NAME]";
    roomDescription = "";
    roomName = "[NONE]";
    hostNicknamePrediction = "";
    location = DEFAULT_LOCATION;
    creationTime = QDateTime::currentDateTime();
    banUserPermission = PERMISSION_EVERYONE;
    roomLatency = -1;

    /* Get ready to receive a signal in case a new location arrives: */
    connect(WebContact::getInstance(), SIGNAL(locationReceived(QString)), this, SLOT(receivedLocation(QString)), Qt::QueuedConnection);

    /* Get ready to receive a signal in case the room was published: */
    connect(WebContact::getInstance(), SIGNAL(roomPublished()), this, SLOT(receivedRoomPublished()), Qt::QueuedConnection);

}

/** GETTERS AND SETTERS: **/

bool RoomDescription::isHostRoom() { return isHost; }
void RoomDescription::setIsHostRoom(bool host) { isHost = host; }

bool RoomDescription::hasPassword() { return isPasswordProtected; }
void RoomDescription::setHasPassword(bool hasPassword) { isPasswordProtected = hasPassword; }

QString RoomDescription::getPassword() { return password; }
void RoomDescription::setPassword(QString roomPassword) { password = roomPassword; }

void RoomDescription::setRoomPort(qint32 port) { roomPort = port; }
qint32 RoomDescription::getRoomPort() { return roomPort; }

void RoomDescription::setRoomIP(QString ip) { roomIP = ip; }
QString RoomDescription::getRoomIp() { return roomIP; }

void RoomDescription::setMaxNumberOfUsers(qint16 max) { maxNumberOfUsers = max; }
qint16 RoomDescription::getMaxNumberOfUsers() { return maxNumberOfUsers; }

void RoomDescription::setTotalUsersInRoom(qint16 totalUsers) { totalUsersInRoom = totalUsers; }
qint16 RoomDescription::getTotalUsersInRoom() { return totalUsersInRoom; }

void RoomDescription::setRoomType(QString name) { roomType = name; }
QString RoomDescription::getRoomType() { return roomType; }

void RoomDescription::setRoomDescription(QString description) { roomDescription = description; }
QString RoomDescription::getRoomDescription() { return roomDescription; }

void RoomDescription::setRoomName(QString name) { roomName = name; }
QString RoomDescription::getRoomName() { return roomName; }

void RoomDescription::setHostNicknamePrediction(QString nickname) { hostNicknamePrediction = nickname; }
QString RoomDescription::getHostNicknamePrediction() { return hostNicknamePrediction; }

void RoomDescription::setLocation(QString geoLocation) { location = geoLocation; }
QString RoomDescription::getLocation()
{
    if (location.isEmpty()) return tr("[Location not revealed]");
    return location;
}

QDateTime RoomDescription::getCreationTime() { return creationTime; }
void RoomDescription::setCreationTime(QDateTime time){ creationTime = time; }

void RoomDescription::setRoomLatency(qint32 latency) { roomLatency = latency; }
qint32 RoomDescription::getRoomLatency() { return roomLatency; }

void RoomDescription::setShowLocation(bool show) { isToShowLocation = show; }
bool RoomDescription::getIsToShowLocation() { return isToShowLocation; }

qint8 RoomDescription::getBanPermission() { return banUserPermission; }
void RoomDescription::setBanPermission(qint8 permission) { banUserPermission = permission; }

bool RoomDescription::IsRoomPublished() { return isPublished; }

QByteArray RoomDescription::getDataPackage()
{
    QByteArray message;
    QByteArray future;
    QDataStream messageStream(&message, QIODevice::WriteOnly);
    getDataHelperMethod(messageStream);

    messageStream << banUserPermission;
    messageStream << roomDescription;
    messageStream << isPublished;
    messageStream << future; //This is a space reserved for future changes.

    addDataToStream(messageStream);
    return message;
}

void RoomDescription::getDataHelperMethod(QDataStream &stream)
{
    stream << roomName;
    stream << roomType;
    stream << maxNumberOfUsers;
    stream << totalUsersInRoom;
    stream << creationTime;
    stream << getLocationToShare();
    stream << isPasswordProtected;
    stream << hostNicknamePrediction;
}

void RoomDescription::setDataHelperMethod(QDataStream &stream)
{
    stream >> roomName;
    stream >> roomType;
    stream >> maxNumberOfUsers;
    stream >> totalUsersInRoom;
    stream >> creationTime;
    stream >> location;
    stream >> isPasswordProtected;
    stream >> hostNicknamePrediction;
}

QString RoomDescription::getLocationToShare()
{
    if (isToShowLocation)
        return location;
    return "";
}

void RoomDescription::setDataPackage(QByteArray message)
{
    QDataStream messageStream(message);
    setDataHelperMethod(messageStream);

    if (!messageStream.atEnd()) { //If there are still data to be read (not basic package)

            QByteArray future;
            messageStream >> banUserPermission;
            messageStream >> roomDescription;
            messageStream >> isPublished;
            messageStream >> future; //This is a space reserved for future changes.

            if (!messageStream.atEnd()) //If there are still data to be read (class extended)
                useDataFromStream(messageStream);
    }
    emit dataUpdated();
}

QByteArray RoomDescription::getBasicDataPackage()
{
    QByteArray message;
    QDataStream messageStream(&message, QIODevice::WriteOnly);
    getDataHelperMethod(messageStream);
    return message;
}

void RoomDescription::receivedLocation(QString location)
{
    if (isHostRoom())
        setLocation(location);
}

void RoomDescription::receivedRoomPublished()
{
    isPublished = true;
}

QByteArray RoomDescription::getFileDataPackage()
{
    QByteArray message;
    QDataStream messageStream(&message, QIODevice::WriteOnly);
    QByteArray package = getDataPackage();
    QByteArray packageCheck = package;
    packageCheck.append("RoomDescription v1.0");
    messageStream << QCryptographicHash::hash(packageCheck,QCryptographicHash::Md5);
    messageStream << qCompress(package);
    return message;
}

bool RoomDescription::setFileDataPackage(QByteArray message)
{
    QDataStream messageStream(message);
    QByteArray package, hashValue;
    messageStream >> hashValue;
    messageStream >> package;
    package = qUncompress(package);
    QByteArray packageCheck = package;
    packageCheck.append("RoomDescription v1.0");
    if (hashValue != QCryptographicHash::hash(packageCheck,QCryptographicHash::Md5))
        return false;
    setDataPackage(package);
    return true;
}

/**
 * Save the current class into a file:
 * @param filePath: the path in the directory where the file will be saved
 */
bool RoomDescription::saveRoomFile(QString filePath)
{
        QFile roomFile(filePath);
        if(!roomFile.open(QIODevice::WriteOnly)) {
            return false;
        } else {
            roomFile.write( getFileDataPackage() );
            roomFile.flush();
            roomFile.close();
            return true;
        }
}

/**
 * Load the current class from a file:
 * @param filePath: the path in the directory where the file will be loaded
 */
bool RoomDescription::loadRoomFile(QString filePath)
{
    QByteArray roomData;
    if(!QFile(filePath).exists())
        qWarning() << "[RoomDescription] File selected does not exist!";
    else {
        QFile roomFile(filePath);
        if(!roomFile.open(QIODevice::ReadOnly))
            qWarning() << "[RoomDescription] File " << filePath << " could not be opened!";
        else
            roomData = roomFile.readAll();
    }

    if (roomData.isEmpty())
        return false;

    return setFileDataPackage(roomData);
}
