#ifndef ROOMVIEW_H
#define ROOMVIEW_H

#include <QWidget>
#include <QSound>

#include "constants.h"

/* Engine: */
#include "../Engine/room.h"
#include "../Engine/roomdescription.h"
#include "../roommember.h"
#include "../roominfo.h"

/* Actions: */
#include "Action/actioncontroller.h"
#include "Action/dialogcontroller.h"
#include "Action/tablecontroller.h"

class DialogController;
class ActionController;
class TableController;
class RoomInfo;
class Table;

namespace Ui {
class RoomView;
}

/**
 * The RoomView is the widget of the app. It is where all the action happens once
 * the user has joined the room. This is the room loaded after the loading screen.
 * In other words, THIS is the room!
 */

#define ZOOM_TABLE_PORCENTAGE 20 //How much % to zoom the table in the Zoom In / Zoom Out buttons.
#define ROTATE_TABLE_ANGLE 90 //How much to rotate the table in the Rotate buttons.

class RoomView : public Room
{
    Q_OBJECT
    
public:
    explicit RoomView(QWidget *parent = 0);
    ~RoomView();

    /**
     ** The types of buttons in the menu bar that
     ** interact with the room view.
     **/
    enum MenuBarButtons {
        BTN_THROW_DICE,
        BTN_ADD_OBJECT,
        BTN_ZOOM_IN,
        BTN_ZOOM_OUT,
        BTN_ROTATE_LEFT,
        BTN_ROTATE_RIGHT,
        BTN_SAVE_TABLE,
        BTN_ROOM_INFO

        /*
         * YOU DEAL WITH THEM IN THE
         * roomButtonClicked() METHOD
         * AND IN THE MainWindow CLASS.
         */
    };

    /* Getters for the main components: */
    RoomMember* user(); // You
    RoomMember* user(qint32 userID);
    RoomMember* hostUser();
    ActionController* action();
    TableController* tableC(); //Table Controller
    RoomInfo* info();
    Table* table();
    QList<User*> users();

    /** Hides the welcome text displayed when the user joins a room: */
    void hideWelcomeText();

protected:
    /** Called before requesting data in the loading screen: */
    bool on_loadingScreen_prepare();

    /** Called when it's time to load the room: */
    bool on_loadingScreen_load();

    /** Called when the room is loaded and it's time to interact with the user: */
    bool on_room_loaded();

    /** Called when the current user is leaving the room: */
    bool on_leaving_room();

    /** Called when somebody joined the room: */
    void on_someone_joined_room(User *user);

    /** Called when somebody leaves the room: */
    void on_someone_left_room(User *user);

    /**
     ** Implementation of the superclass method to deal with buttons pressed in the menu bar:
     ** @param index: what button was clicked (enum MenuBarButtons).
     **/
    void roomButtonClicked(int index);

    /** Retranslates the app if its language changed: */
    void changeEvent(QEvent *event);

    /**
     ** Called when you receive a message from the server.
     ** @param message: the message sent that should be opened.
     ** @param user: who sent the message.
     ** @param type: the type of message (enum ActionMessageTypes)
     ** Use method sendActionMessage() to send a message to the server.
     **/
    void actionMessageReceived(QByteArray message, User *user, qint8 type);

    /**
     ** Called when you receive random values from the server.
     ** @param min, max: the boundaries of the random values
     ** @param user: who requested the values.
     ** @param type: the type of random values (enum RandomMessageTypes)
     ** Use method requestRandomValues() to request rand values from the server.
     ** Random values are better computed server-side to prevent cheating.
     **/
    void randomValuesReceived(qint32 min, qint32 max, qint16 type, QList<qint32> values, User *user);

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
    void large_message_started(qint8 type,  QByteArray info, qint32 size, User *user);
    void large_message_progress(qint8 type, QByteArray info, double percentage, User *user) ;
    void large_message_cancelled(qint8 type, QByteArray info, User *user);
    void large_message_finished(qint8 type, QByteArray info, QByteArray package, User *user);

private slots:
    /** Saves the table file periodically (called by the autosave timer): */
    void autosaveTableFile();

    /** Called when your team changed (to update the view and permissions): */
    void onTeamChanged(RoomMember* memberChanged, RoomMember* memberWhoChanged);

    /**  Called when your admin status changed (to update the view and permissions): */
    void onAdminStatusChanged(User* memberChanged, User* memberWhoChanged);

    /** Called by the timer after the view is loaded to allow resizing of the window: */
    void allowResizing();

private:

    /** Connect signals and slots for the table controller: */
    void tableControllerMakeConnections();

    /** Connect signals and slots for the action controller: */
    void actionControllerMakeConnections();

    /** Connect signals and slots for the dialog controller: */
    void dialogControllerMakeConnections();

    /** Connect signals and slots for the user that represents you: */
    void userMakeConnections();

    /**
     * Display a welcome chat message to whoever joins the room.
     * This message should have the room text description.
     */
    void displayWelcomeChatMessage();

    /** When the user joins the room, start a timer to periodically save the room file: */
    void startAutosaveTimer();

    /** Set up what you can do based on permissions you might have: */
    void updatePermissions();

    /** Assign a team for a member as soon as he/she enter the room: */
    void assignTeam(RoomMember *user);

    /* Private variables: */
    Ui::RoomView *ui;
    RoomMember *member; //A pointer to the user.
    RoomInfo *roomInfo; //All the data about the room should be there.
    ActionController *actionController; //An object to deal with the messages.
    TableController *tableController; //An object to deal with table messages.
    DialogController *dialogController; //Deals with the dialogs in the screen

    /* Timer to autosave the table: */
    QTimer autosave;
};

#endif // ROOMVIEW_H
