#ifndef ROOM_H
#define ROOM_H

#include <QWidget>
#include <QDebug>
#include <QListWidgetItem>

#include "Engine_global.h" /* This class can be used outside the Engine */
#include "roomcontroller.h"
#include "filetransfer.h"

/* Make this class "half" aware of the following Engine classes: */
class RoomController;
class User;
class FileTransfer;


/**
  @brief This class is the engine behind
  the room where all the action happens.
  It is loaded in the loading screen and
  called after that. The details about
  it is implemented outside the Engine.
*/


class ENGINESHARED_EXPORT Room : public QWidget
{
    Q_OBJECT
public:
    explicit Room(QWidget *parent = 0);

    /**
     ** Messages that can popup depending on how the user leaves a room:
     ** This is being used by the ViewController and Room:
     ** === THERE'S A COPY OF THIS ENUM AT VIEWCONTROLLER ===
     **/
    enum ReturnMessages {
        MSG_NO_MESSAGE,
        MSG_BANNED,
        MSG_DISCONNECTED,
        MSG_DROPPED,
        MSG_SERVER_CLOSED
    };

    /*
     * RoomController should be set after initializing this class.
     * This is not in the constructor because classes that will
     * inherit this one must not have access to this pointer.
     */
    void setRoomController(RoomController *controller);

    /**
     * @return true if the user is currently seeing the room.
     */
    bool userInRoom();

    /**
     * @return true if the room was loaded.
     */
    bool isRoomLoaded();

    /**
     * @return the number of users that are currently in the room (including you).
     */
    int totalUsersInRoom();

    bool prepareRoom(); //Called before requesting info to the server.
    bool loadRoom(); //Called in the loading screen to prepare room
    bool startRoom(); //Called to show the Room

signals:

    /*
     * Open any dock widget. This can be used in multiple ways depending on the type of
     * application using this class:
     */
    void openDockWidget(QDockWidget *widget, Qt::DockWidgetArea area, bool isFloating = false);

    /* Tabify a dock widget (putting one over the other): */
    void tabifyDockWidget(QDockWidget *first, QDockWidget *second);

    /* Just call leaveRoom() instead of this signal: */
    void goBackToMenu(qint8 message);

    /* Emit a signal asking the main window of the application to enable/disable a button: */
    void buttonSetEnabled(int button, bool enabled);

    /*
     * These signals are only used locally and is to send to an inner class in the Engine:
     * Don't use these signals. It's better to call the method sendLargeMessage() instead
     * of directly sending this signal.
     */
    void sendP(QByteArray p1, QByteArray p2, User *u);
    void sendF(QString p1, QByteArray p2, User *u);

public slots:
    /**
      * Called when the user wants to leave room. The View Controller is
      * the class that is likely to call this slot.
      */
    void leaveRoom(ReturnMessages type = MSG_NO_MESSAGE);

    /**
      * Slot connected to the signal roomButtonClicked() in the View Controller.
      * That signal might be emitted when a button outside the Room is clicked
      * and the content of this button is relevant to the Room. Eg: The Menu Bar
      * that has to be outside the Room in the main window of the application.
      * This function can be implemented outside the Engine.
      * @param index: what button was clicked (to be defined outside the Engine).
      */
    virtual void roomButtonClicked(int) { }

    /** You can implement that. This function is called when a dock widget is opened. */
    virtual void dockWidgetOpened(QDockWidget*) { }

    /** You can implement that. This function is called when the user selects a row in the action log: */
    virtual void logSelected(QModelIndex) { }

    /** Add an item to the action log: */
    bool log(QListWidgetItem *logItem);

    /** Add a text item to the action log: */
    bool logText(QString text, QColor color);


protected:

    /**
        This function is called by the loading screen asking to prepare room BEFORE data has arrived.
        Remember that this function is called before knowing whether or not the user will be allowed
        to join the room. So, be careful because if the user can't join the room, this class will be
        deleted right after.
        @return true if the room was successfully prepared.
    */
    virtual bool on_loadingScreen_prepare() = 0;

    /**
        This function is called by the loading screen asking to load room AFTER data has arrived.
        This is where all the heavy loading is done. It can take a while here loading data.
        Also, at this point you are not inside the Room yet. So, you can't have your User object
        yet, and if you request the list of users in the room, you won't be there.
        @return true if the room was successfully loaded.
    */
    virtual bool on_loadingScreen_load() = 0;

    /**
        This function is called by the mainMenu when the room is loaded.
        You can assume from here that the user is already inside the room
        and all the connections to send network messages are set. The
        computation of this class is still done inside the loading screen,
        but the room view is already loaded.
        @return true if the room started well.
    */
    virtual bool on_room_loaded() = 0;

    /** Called when you are leaving the room: */
    virtual bool on_leaving_room() = 0;

    /** Called when somebody joins the room: */
    virtual void on_someone_joined_room(User *user) = 0;

    /** Called when somebody leaves the room: */
    virtual void on_someone_left_room(User *user) = 0;

    User* getCurrentUser(); //You
    User * getUser(qint32 userID);
    User* getHostUser();  //Who is hosting the room
    QList<User*> getUsersInRoom(); //All users in the room
    RoomDescription* getRoomDescription(); //Info about the room

protected slots:

    /**
     * Add a chat message to the chat (only for you).
     * @param defaultColor: the color of the message (optional)
     * @return true if it was added: */
    bool addChatMessage(QString message, QColor color = "");

    /* Action messages: */
    void sendActionMessage(qint8 type, QByteArray message, User *user = 0, bool isTCP = TCP_MESSAGE);
    void sendActionMessageToHost(qint8 type, QByteArray message, bool isTCP = TCP_MESSAGE);
    virtual void actionMessageReceived(QByteArray message, User *user, qint8 type) = 0;

    /* Sending large messages through the network: */
    void sendLargeMessage(qint8 type, QByteArray largePackage, QByteArray info, User *user = 0);
    void sendFile(qint8 type, QString filepath, QByteArray info, User *user = 0);

    /**
     ** Called when you receive a large message from the server. Differently from other messages,
     ** a large message calls "message_started" to warn you that there's a large message on the way.
     ** "message_progress" is called periodically warning you about the percentage downloaded. Then,
     ** "message_finished" is the last message called with the full package. If something happens
     ** (such as whoever is sending the message leaves the room), "message_cancelled" will be called.
     ** @param info: a small personal message sent together with the large message. This message is
     ** just sent once and can be used to identify what is being downloaded.
     ** @param user: who is sending the message.
     ** @param package: the big message that was downloaded.
     ** Use method sendLargeMessage() to send a large message to the server. You can also
     ** send a large message by calling the method sendFile().
     **/
    virtual void large_message_started(qint8 type,  QByteArray info, qint32 size, User *user) = 0;
    virtual void large_message_progress(qint8 type, QByteArray info, double percentage, User *user) = 0;
    virtual void large_message_cancelled(qint8 type, QByteArray info, User *user) = 0;
    virtual void large_message_finished(qint8 type, QByteArray info, QByteArray package, User *user) = 0;

    /**
     * Request a random value from the server.
     * Function randomValueReceived() is called when it is received.
     * @param count: total of random values to be generated (within min and max). There's limit of about a thousand.
     * @param type: just an integer to help you identify the request. No use in the engine.
     * @param values: the list of all generated random numbers.
     * @param user: the user who requested the numbers.
     */
    void requestRandomValues(qint32 min, qint32 max, qint16 type, qint32 count = 1, User *to = 0, bool isTCP = TCP_MESSAGE);
    virtual void randomValuesReceived(qint32 min, qint32 max, qint16 type, QList<qint32> values, User *user) = 0;

private slots:
    /* Receive large messages (this includes files): */
    void largeMessageStarted(QByteArray info, qint32 size, User *user);
    void largeMessageProgress(QByteArray info, double percentage, User *user);
    void largeMessageCancelled(QByteArray info, User *user);
    void largeMessageFinished(QByteArray info, QByteArray package, User *user);

    void banned();
    void leftRoom();
    void serverClosed(bool isClosed);
    void disconnectedFromServer(bool isDisconnected);
    void messageReceived(QByteArray package, User *user);
    void randomReceived(qint32 min, qint32 max, qint16 type, QList<qint32> values, User *user);
    void someoneConnected(User* user);
    void someoneDisconnected(User* user);

private:
    bool hasLeftRoom;
    bool isUserInRoom;
    bool roomLoaded; //true if the room was loaded.
    RoomController *roomController;
    FileTransfer *fileTransfer;
};

#endif // ROOM_H
