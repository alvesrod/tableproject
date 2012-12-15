#ifndef USER_H
#define USER_H

#include <QObject>
#include <QTime>
#include <QDebug>
#include <QColor>
#include <QStringList>
#include "packagehandlerinterface.h"
#include "usersettings.h"
#include "Engine_global.h" /* This class can be used outside the Engine */

/* Make this class "half" aware of the following Engine classes: */
class UserSettings;

/**
  * @brief
  * This class stores the generic information about every user in the room. It is
  * a class that has to be extended outside the Engine to implement more precise
  * info about a user. This also handles messages to edit user information (sent
  * through the network).
  */

class ENGINESHARED_EXPORT User : public QObject, public IPackageHandler
{
    Q_OBJECT

public:

    /** Types of messages that handleMessage() can handle: */
    enum UserMessageTypes {
       /*  500 */ MSG_CHANGE_NICKNAME = 500, //500: to prevent conflict with inherited classes.
       /*  501 */ MSG_CHANGE_CHAT_COLOR,
       /*  502 */ MSG_CHANGE_ADMIN_STATUS

    };

    User(qint32 id, QObject *parent, UserSettings *settings, bool isUserHost = false);

    /**
     * @brief sendEditUserMessage sends through the network a message
     * related to this particular user (such as change username).
     * @param type (from enum UserMessageTypes or any new type
     * defined in the class that extends User).
     */
    void sendEditUserMessage(qint16 type);

    /**
     * Called when you request the latency for a specific user. This function will
     * only start the countdown to be able to measure how many miliseconds it took for
     * the message to come back.
     */
    void latencyRequested();

    /**
     * Called when the message came back from the latency request. This function will
     * only stop the countdown and return how many miliseconds it took from when
     * latencyRequested() was called to when getLatency() was called.
     */
    qint16 getLatency();

    /** @return the number of seconds since this user class was created (when the user joined the room). */
    qint32 getSecondsSinceCreated();

    /**
     *  Return true if the user has permission to do something (based on the permission index).
     *  This function does not check if the user is the current user, which can affect the
     *  permission. eg: A user cannot ban himself or send a private message to himself.
     */
    bool hasPermission(qint8 permissionIndex, bool isOwnerOfObject = false);

    /*
     * Those 2 methods are used for the network to prepare user messages.
     * That is, messages that will do something with this user, such as edit a user variable.
     */
    void handleMessage(QByteArray message, User *editor);
    QByteArray prepareMessage(qint16 type);


    /**
     * @brief makeNameUnique prepares the nickname sufix variable in
     * order to make the nickname unique (by putting a number in the
     * end of the name if there are multiple users with the same name).
     * @param allMembers the list of all users in the room to be
     * able to compare nicknames.
     */
    void makeNameUnique(QList<User *> allMembers);

    /** Erase the nickname sufix that was set through the makeNameUnique() method. */
    void resetNicknameSufix();

    /*
     * Some getters and setters for user variables:
     * Some of them you might avoid changing outside the Engine.
     */

    bool isUserOnline();
    void setIsOnline(bool online); //Already set inside the Engine when the user joins/leaves a room.

    bool isThatYou();
    void setIsYou(bool you); //Already set inside the Engine when the user joins a room.

    bool isTheHost();
    void setIsHost(bool host); //Already set inside the Engine when the user hosts a room.

    bool isAnAdmin();
    void setIsAdmin(bool admin);

    qint32 getUniqueID();
    void setUniqueID(qint32 id);

    QString getNickname(bool appendSufix = true);
    void setNickname(QString name);

    QColor getMemberChatColor();
    void setMemberChatColor(QColor color);

    UserSettings* getUserSettings();
    void setUserSettings(UserSettings *settings);

    /** @return when the user joined the room: */
    QTime getCreationTime();

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

    /** @return true if the @param user is the same as this user */
    bool isEqual(User* user);

    /*
     * Overloading the == and != operator. == returns true if users have the same id
     * and were created at the same date.
     */
    bool operator==(const User& other) const;
    bool operator!=(const User& other) const;

signals:
    /* Called when the specific variable is changed: */
    void nicknameChanged(User* memberChanged, User* memberWhoChanged, QString oldName);
    void chatColorChanged(User* memberChanged);
    void adminStatusChanged(User* memberChanged, User* memberWhoChanged);

    /** Signal emmited when this class was updated based on a network package: */
    void dataUpdated();

protected:

    /**
     * Add a chat message to the chat (only for you).
     * @param defaultColor: the color of the message (optional)
     * @return true if it was added: */
    bool addChatMessage(QString message, QColor color = "");

    /*
     * To be implemented by the class that inherits User.
     * Those methods deal with the network messages. You can
     * add or remove variables from the @param stream. Check
     * the examples to see exactly how they work.
     */
    virtual void onMessageReceived(QDataStream &stream, qint16 type, User *editor) = 0;
    virtual void onMessageRequest(QDataStream &stream, qint16 type) = 0;

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

private slots:
    void chatColorChanged(QColor);
    void nicknameChanged(QString name);

private:
    QString getNicknameSuffix(qint8 number);
    bool nameConflict(QString name, QList<User*> allMembers);

    UserSettings *userSettings; //Stores settings like the username and chat color.
    RoomController *roomController;
    qint32 uniqueID; //Each member has one
    QTime creationTime; //Records the time the object from this class was created (when the user joined the room).
    qint8 nicknameSufix; //the number in the username if there are 2 users with the same name in the room.
    QTime pingTime; //to record the ping
    qint16 latestPing; //to record the ping (latency).

    bool isOnline; //true when the user is online
    bool isYou; //true when this is you
    bool isHost; //true when this is the person who hosted the room.
    bool isAdmin; //true when this is an administrator (the application will decide who is admin).
};

#endif // USER_H
