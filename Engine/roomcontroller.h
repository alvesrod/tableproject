#ifndef ROOMCONTROLLER_H
#define ROOMCONTROLLER_H

/* Qt libraries: */
#include <QDateTime>

#include "Network/network.h" /* RoomController is the only one that accesses the network: */
#include "roomdescription.h"
#include "user.h"
#include "usersettings.h"
#include "Views/chatroom.h"

/* Make this class "half" aware of the following Engine classes: */
class LoadingScreen;
class User;
class UserSettings;
class ChatRoom;
class RoomDescription;

/**
  @brief This class controls the current room, such as who is hosting it, trade messages
  with other members, etc. It is the class that deals with the communication.
  So, all the contact to the network will be found here only.

  This class will translate the main packages from the network and do the main
  organization. When a package is specific about something (like changing a user variable),
  the RoomController will simply delegate the package to the other class (eg: the user class).

  ================
  ATTENTION:
    AS PART OF THE ENGINE, THIS CLASS MUST NOT KNOW ANYTHING ABOUT THE PURPOSE OF
    THE APPLICATION! THIS IS ESSENTIAL FOR ITS REUSABILITY. THE ENGINE CAN BE USED
    IN MANY DIFFERENT MULTIUSER APPLICATIONS. ALSO, THIS IS A DELICATE CLASS BECAUSE
    IT DEALS WITH THE NETWORK MESSAGES AND VALIDATION. SO, TO PREVENT CHEATING,
    THIS CLASS SHOULD BE HIDDEN FROM ANY CLASS OUTSIDE THE ENGINE LIBRARY. THIS CANNOT
    (AND MUST NOT) BE ACCESSED OUTSIDE THE ENGINE.
  ================
 */

#define TO_EVERYONE NULL //Send network message to everyone
#define MAX_NUMBER_OF_DISCONNECTS 10 //How far the controller registers the people who got disconnected.
#define TIMEOUT_WAITING_DISCONNECT_SECONDS 12 //Total of seconds waiting a message from someone who disconnected
#define MAX_NUMBER_OF_PING_USERS 50 //Ping users are the users who are just getting info about the room (not joining them).
#define MAX_RANDOM_VALUES_COUNT 2000 //When the user request a random value from the server, this is the limit of values in 1 request.
#define DISPLAY_RC_DEBUG_MESSAGES false

class RoomController : public QObject
{
    Q_OBJECT

public:
    explicit RoomController(RoomDescription* description, UserSettings* settings = NULL);
    ~RoomController();

    /** Types of messages that can be sent to network: */
    enum NetworkMessageTypes {
       /*  0 */ MSG_PREPARE_ROOM, //Asks for all the information needed to prepare the room before entering one.
       /*  1 */ MSG_PREPARE_ROOM_REPLY, //The reply sent by the host with all the room details (loading screen).
       /*  2 */ MSG_ROOM_PREPARED, //Sent by a user when he is done loading the room.
       /*  3 */ MSG_REQUEST_ROOM_INFO, //Ask for room info, such as latency, room name, total of players in it, etc.
       /*  4 */ MSG_REQUEST_ROOM_INFO_REPLY, //The reply sent by the host with the room info.
       /*  5 */ MSG_REMOVE_USER, //Removes the user from the room politely (due to error).
       /*  6 */ MSG_BAN_USER, //Bans the user from the room (and adds the user's ip to the blacklist).
       /*  7 */ MSG_PING, //Just an empty message used to measure the latency.
       /*  8 */ MSG_PING_REPLY, //The reply from the ping message.
       /*  9 */ MSG_CHAT_MESSAGE, //Sends a chat message
       /* 10 */ MSG_PRIVATE_CHAT_MESSAGE, //Sends a crypted private chat message
       /* 11 */ MSG_EDIT_ROOM_MEMBER, //Edit some data about a room member (handle by the Room member)
       /* 12 */ MSG_ACTION, //Action message (handled by the Action Controller)
       /* 13 */ MSG_ACTION_HOST, //Action message to the host user (handled by the Action Controller)
       /* 14 */ MSG_DOWNLOAD, //Deals with large packages to be downloaded (handled by the File Transfer class)
       /* 15 */ MSG_RANDOM_VALUE, //Asks the host to generate a random value based on min and max
       /* 16 */ MSG_RANDOM_VALUE_REPLY, //Reply sent from the host with the random value generated
       /* 17 */ MSG_OTHER //Other message (avoid using that type)

        /**
          *  THOSE ARE ALL THE TYPES. Try not to add more types.
          *  Usually other types are just a subtype of MSG_ACTION
          */
    };

    /** Types of answers that a user can receive while trying to join the room: */
    enum NetworkPrepareRoomAnswers {
        ANSWER_ALLOWED, //User is allowed to enter in the room.
        ANSWER_ROOM_FULL, //User cannot enter because the room is already full.
        ANSWER_WRONG_VERSION, //User cannot enter because his app is in a different version.
        ANSWER_WRONG_APP_KEY, //User cannot enter because the app secret key didn't match (modified application).
        ANSWER_WRONG_PASSWORD, //User cannot enter because the password entered is wrong.
        ANSWER_DISCONNECTED_RECENTLY, //User was disconnected too recently. Host is unsure if he is really there.
        ANSWER_BANNED, //User is in the blacklist and cannot join the room
        ANSWER_DENIED_OTHER //User is not allowed for other possible reasons
    };

    /** Add a chat message to the chat (only for you). Returns true if it was added: */
    bool addChatMessage(QString message, QColor color = "");

    /** Add an action log message to the action log (only for you).  Returns true if it was added: */
    bool addActionLog(QListWidgetItem *logItem);

    /**
     * Request random values that will be computed in the host and returned to @param to.
     * When the values return, this class will emit randomValuesReceived().
     * @param type is just a number used for whatever calls this method to identify the value
     * requested.
     */
    void requestRandomValues(qint32 min, qint32 max, qint16 type, qint32 count = 1, User *to = TO_EVERYONE, bool isTCP = TCP_MESSAGE);

    /**
     * Grabs a package and cryptographs it. If the package is already crypted,
     * it will decrypt it. The crypt is based on 2 keys. The localKey and one
     * key received from the host.
     */
    QByteArray crypt(QByteArray package, QByteArray localKey = "abc", bool useHostKey = true);

    /** GETTERS AND SETTERS: **/
    QHash<qint32, User *> getListOfUsers();
    QList<User *> getPrimitiveListOfUsers();
    User* getCurrentUser();
    User* getHostUser();
    RoomDescription* getRoomDescription();
    bool isHost();
    UserSettings* getUserSettings();
    void setPassword(QString password);
    QString getPassword();
    void setChatroom(ChatRoom *chat);
    ChatRoom* getChatroom();
    User* searchUser(qint32 id);


    /** NETWORK PUBLIC METHODS: **/
    void startNewServer();
    void connectToServer();
	void closeServer();
    void disconnectFromServer();
    void leaveRoom();
    void sendMessageToServer(NetworkMessageTypes type, QByteArray message = NULL, User *to = TO_EVERYONE, bool isTCP = TCP_MESSAGE);

signals:
    void roomPrepared(bool success, QString result = ""); //sent during loading screen.
    void roomHasInfoReady(); //sent when the room already loaded all the info from the REQUEST_ROOM_INFO message.
    void removePublishedRoom(qint32 port); //sent when the host closes the room.

    /** NETWORK SPECIFIC PUBLIC SIGNALS (TO SEND TO OTHER CLASSES): **/
    void genericMessageReceived(QByteArray message, User *user);
    void chatMessageReceived(QByteArray message, User *user);
    void downloadMessageReceived(QByteArray message, User *user);
    void privateChatMessageReceived(QByteArray message, User *user);
    void actionMessageReceived(QByteArray message, User *user);
    void randomValuesReceived(qint32 min, qint32 max, qint16 type, QList<qint32> values, User *user);
    void errorMessage(QString error);
    void disconnectedFromServer(bool success = true);
    void serverStarted(bool serverStarted);
    void connectedToServer(bool connectedToServer);
    void messageSent(bool messageSent);
    void serverClosed(bool serverClosed);
    void someoneConnectedToServer(User *user);
    void someoneDisconnectedFromServer(User *user);
    void leftRoom();
    void pingReceived(User *user);
    void banned();


    /** NETWORK SPECIFIC PRIVATE SIGNALS (TO SEND TO THE NETWORK THREAD ONLY): **/
    void networkStartNewServer(qint32 port);
    void networkConnectToServer(QString ip, qint32 port);
    void networkSendMessageToServer(QByteArray message, qint32 to, bool TCP = true);
    void networkCloseServer();
    void networkDisconnectFromServer();

public slots:
    /** NETWORK PUBLIC SLOTS: **/
    void sendDownloadMessageToServer(QByteArray message = NULL, User *to = TO_EVERYONE);

    /** BELOW IS JUST PRIVATE STUFF. IT DOESN'T MATTER: */
private slots:
    void sendMessageToServerByID(NetworkMessageTypes type, QByteArray message = NULL, qint32 msgTo = BROADCAST_DESCRIPTOR, bool isTCP = true);
    void networkMessageReceived(qint32 sender, QByteArray package); //Send when a message was received.
	void networkErrorMessage(QString error);
    void networkDisconnectedFromServer(bool success = true);
    void networkServerStarted(bool started, quint16 port);
    void networkConnectedToServer(bool connected);
    void networkMessageSent(bool sent);
    void networkServerClosed(bool closed);
    void someoneDisconnected(qint32 descriptor);

private:
    void setupNetworkConnections();
    bool addUser(User *user);
    bool isRoomFull();
    QByteArray getPrepareRoomMessage();
    QByteArray getRoomPreparedMessage();
    QByteArray getPreparePingMessage();
    void prepareRequestRoomInfoMessage(qint32 id);
    void sendPackage( qint32 to, QByteArray package, bool TCP );
    bool getSenderAndCheckMessage(qint32 sender, User *&senderProfile, qint8 type );
    void prepareRoomReadMessage(User *senderProfile, QByteArray message, qint32 sender );
    void preparePingMessage( QByteArray message );
    void preparePingMessageReply( QByteArray message );
    void prepareRoomReplyReadMessage(qint32 sender, QByteArray message );
    void prepareUserMessage(QByteArray message, User *senderProfile);

    void prepareRandomValueMessage(QByteArray message, User *senderProfile);
    void prepareRandomValueMessageReply(QByteArray message, User *senderProfile);

    void prepareRoom(qint32 sender, QDataStream &messageStream );
    void roomPreparedMessage(QByteArray message, qint32 sender );
    bool wasUserRecentlyDisconnected(qint32 userID);
    void removeUserFromComingUsersArray(qint32 id);
    void dealWithGenericMessageReceived(QByteArray message, User *user);
    void addToPingUsersList(qint32 id);
    bool isInPingUsersList(qint32 id);
    void addUserToLatestUnknownDisconnects(qint32 id);
    bool makeSureUserSettingsIsDeclared();
    void addUserToBannedList(qint32 id);
    QByteArray generateRoomCryptKey();
    QByteArray getSecretApplicationKey(QString userIP);
    QByteArray privateMessageCrypt(QByteArray message, qint32 id1, qint32 id2);

    /** Private Variables: **/
    bool hasLeftRoom; //true when the user already left the room.
    qint32 totalOfGenericMessages; //count the number of generic messages received.
    RoomDescription *roomDescription;
    User* currentUser; //a pointer to you!
    User* hostUser; //a pointer to who is hosting the room.
    qint32 currentUserID; //your id
    QHash<qint32, User*> users; //users in the Room (also including you)
    QHash<QString, User*> bannedUsers; //users in the blacklist (banned from the room).
    QList<qint32> comingUsers; //the users that are currently in the loading screen.
    QList<qint32> pingUsers; //recent users that ping the room
    QList<User*> latestUnknownDisconnects; //keep track of the latest users disconnected in case you receive a msg from a user leaving the room before joining a room.
    Network *networkThread;
    UserSettings *userSettings;
    ChatRoom *chatroom;
    QByteArray generatedKey;
    
};

#endif // ROOMCONTROLLER_H
