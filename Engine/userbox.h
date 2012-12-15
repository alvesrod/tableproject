#ifndef USERBOX_H
#define USERBOX_H

#include <QWidget>
#include <QTimer>
#include "Engine_global.h" /* This class can be used outside the Engine */
#include "user.h"

/* Make this class "half" aware of the following Engine classes: */
class User;

#define LATENCY_UPDATE_INTERVAL_SECONDS 8

/**
 * @brief The UserBox class implements the engine behind the
 * dialog that represents a specific user (called through the
 * list of users in the chat). It is in this class that an user
 * can make an user admin, see latency, and other forms of
 * user interaction through a GUI. The class that extends this one
 * will implement the GUI and more specific methods for user interaction.
 */

class ENGINESHARED_EXPORT UserBox : public QWidget
{
    Q_OBJECT
public:
    explicit UserBox(User *user, QWidget *parent = 0);
    ~UserBox();

    /**
     * RoomController should be set after initializing this class.
     * This is not in the constructor because classes that will
     * inherit this one must not have access to this pointer.
     */
    void setRoomController(RoomController *controller);

    /** Called when the user leaves, but the userbox is still opened: */
    void userLeft();

    /** Called when the user leaves, but the userbox is still opened (to be implemented
     * by the class that inherits this one outside the Engine): */
    virtual void on_user_left() = 0;

protected:

    /** Called when there's a new latency for this user: */
    virtual void displayLatency(int latency) = 0;

    /**
     * To be implemented. The UI should refresh its values when this function is called. It
     * will be called usually when some value changed.
     */
    virtual void refreshUI() = 0;

    /**
     * Make this user admin. The message will be sent through the network,
     * so that others will know there's a new admin.
     */
    void setAdminStatus(bool isAdmin);

    /** Add a chat message to the chat (only for you). Returns true if it was added: */
    bool addChatMessage(QString message, QColor color = "");

    /* Getters: */
    User* getUser();
    RoomDescription* getRoomDescription();
    User* getCurrentUser(); //you

protected:

    /** Called when the user opens the UserBox dialog: */
    virtual void showEvent(QShowEvent *);

    /** Called when the user closes the UserBox dialog: */
    virtual void hideEvent(QHideEvent *);

private slots:
    void pingReceived(User *user);
    void nicknameUpdateFromUser(User* userChanged,User* , QString);
    void adminStatusUpdateFromUser(User* userChanged,User*);
    void adminStatusChanged(User* userChanged,User*);
    void refreshLatency();

private:
    void constantLatencyUpdate();

    User *user;
    RoomController *roomController;
    QTimer *latencyUpdateTimer;
    
};

#endif // USERBOX_H
