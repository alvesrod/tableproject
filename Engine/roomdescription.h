#ifndef ROOMDESCRIPTION_H
#define ROOMDESCRIPTION_H

#include <QObject>
#include <QDebug>
#include <QDateTime>
#include <QDataStream>
#include "Engine_global.h" /* This class can be used outside the Engine */
#include "packagehandlerinterface.h"
#include "webcontact.h"

/**
 * @brief
 * This class stores the generic information about each room. It is a class
 * that has to be inherited by a class outside the Engine. The inheritance
 * will implement its virtual methods and deal with precise information
 * about each room.
 */

class ENGINESHARED_EXPORT RoomDescription : public QObject, public IPackageHandler
{
    Q_OBJECT
public:
    RoomDescription(qint32 port, QString ip = LOCAL_IP, QObject *parent = 0);

    enum Permissions {
        PERMISSION_HOST_ONLY, //Who hosted the room
        PERMISSION_ADMINS_ONLY, //Who was marked as admins by the host
        PERMISSION_EVERYONE, //Everyone
        PERMISSION_NOBODY, //Nobody
        PERMISSION_YOURS_ONLY //Who owns whatever is being required permission (eg: an object)
    };
    
    /** GETTERS AND SETTERS: **/
    bool isHostRoom();
    void setIsHostRoom(bool host);
    bool hasPassword();
    void setHasPassword(bool hasPassword);
    QString getPassword();
    void setPassword(QString roomPassword);
    void setRoomPort(qint32 port);
    qint32 getRoomPort();
    void setRoomIP(QString ip);
    QString getRoomIp();
    void setMaxNumberOfUsers(qint16 max);
    qint16 getMaxNumberOfUsers();
    void setTotalUsersInRoom(qint16 totalUsers);
    qint16 getTotalUsersInRoom();
    void setRoomType(QString name);
    QString getRoomType();
    void setRoomDescription(QString description);
    QString getRoomDescription();
    void setRoomName(QString name);
    QString getRoomName();
    void setHostNicknamePrediction(QString nickname);
    QString getHostNicknamePrediction();
    void setLocation(QString geoLocation);
    QString getLocation();
    QDateTime getCreationTime();
    void setCreationTime(QDateTime time);
    void setRoomLatency(qint32 latency);
    qint32 getRoomLatency();
    void setShowLocation(bool show);
    bool getIsToShowLocation();
    qint8 getBanPermission();
    void setBanPermission(qint8 permission);
    bool IsRoomPublished();

    /*
     * Methods from the PackageHandler interface that this class implements.
     * Those methods deal with all the shared class variables as a single ByteArray
     * package (necessary to send through the network). The operators << and >>
     * are overloaded to call those 2 functions. One prepares the package based on the
     * values of the variables. The other reads the package in the parameter and assign
     * values to the shared variables based on that package:
     */
    QByteArray getDataPackage();
    void setDataPackage(QByteArray message);

    /**
      * BasicDataPackage is like getDataPackage, but it doesn't include specific data about the room:
      * It is called by the RoomController when just pinging a room for more information:
      */
    QByteArray getBasicDataPackage();

    /*
     * Use those methods to save and load the variables of this class into a file.
     * The variables of classes that inherits this one might also be loaded if they
     * are inside the function "addDataToStream." These functions
     * use a md5 hash to make sure the user did not edit the information.
     */
    bool saveRoomFile(QString filePath);
    bool loadRoomFile(QString filePath);


signals:
    /** Signal emmited when this class was updated based on a network package: */
    void dataUpdated();

protected:
    /*
     * To be implemented by the class that inherits this one.
     * addDataToStream() is just inside getDataPackage() to add
     * more data to the package (such as new variables not implemented
     * in this class). useDataFromStream() is inside setDataPackage() and
     * behaves like it. The difference is that those 2 functions are implemented
     * by the class that inherits this one while getDataPackage() and setDataPackage()
     * are only implemented by this class.
     */
    virtual void addDataToStream(QDataStream &stream) = 0;
    virtual void useDataFromStream(QDataStream &stream) = 0;


    /* Private stuff below: */

private slots:
    void receivedLocation(QString location);
    void receivedRoomPublished();

private:
    void getDataHelperMethod(QDataStream &stream);
    void setDataHelperMethod(QDataStream &stream);
    QString getLocationToShare();

    /*
     * The get and set FileDataPackage works exactly like the get and set
     * DataPackage. The difference is that this one also includes a hash
     * of the variables to make sure they are accurate (and prevent the user
     * to manually edit information). This one is to save the Room Description on disk.
     */
    QByteArray getFileDataPackage();
    bool setFileDataPackage(QByteArray message);

    /** Private variables that stores generic info about the room: */
    bool isHost;
    bool isPasswordProtected;
    bool isToShowLocation;
    bool isPublished;
    QString password;
    qint32 roomPort;
    QString roomIP;
    qint16 maxNumberOfUsers;
    qint16 totalUsersInRoom;
    QString roomType;
    QString roomDescription;
    QString roomName;
    QString hostNicknamePrediction;
    QString location;
    QDateTime creationTime;
    qint32 roomLatency;
    qint8 banUserPermission;
    
};

#endif // ROOMDESCRIPTION_H
