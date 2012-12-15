#ifndef VIEWCONTROLLER_H
#define VIEWCONTROLLER_H

#include <QMainWindow>
#include <QDockWidget>
#include <QMessageBox>
#include <QDebug>
#include <QSound>
#include "Engine_global.h" /* This class can be used outside the Engine */
#include "usersettings.h"

/* Views: */
#include "menu.h"
#include "room.h"
#include "loadingscreen.h"
#include "Views/chatroom.h"

/* Make this class "half" aware of the following Engine classes: */
class RoomController;

/**

  @brief This is the class that holds all the views. So, the switching of views
  happens here. It stores a pointer to the Room Controller and the User Settings
  just to send them through different views.

  The main role of this class is to swith views, store important pointers, and
  control the menu and status bar.

  It should be inherited by the Main Window class of the application.
  By being part of the Engine, this class must know nothing about the purpose of the app.

 */

/* The location of the chat box when the room opens: */
#define CHAT_BOX_AREA 0x8 //Qt::BottomDockWidgetArea (bottom of the screen)

class ENGINESHARED_EXPORT ViewController : public QMainWindow
{
    Q_OBJECT
public:
    explicit ViewController(QWidget *parent = 0);
    ~ViewController();

    /**
     ** Messages that can popup depending on how the user leaves a room:
     ** This is being used by the ViewController and Room:
     ** === WARNING: THERE'S A COPY OF THIS ENUM AT ROOM ===
     ** (Sadly, Qt didn't allow me to just reference this enum. I had to make a copy).
     **/
    enum ReturnMessages {
        MSG_NO_MESSAGE,
        MSG_BANNED,
        MSG_DISCONNECTED,
        MSG_DROPPED,
        MSG_SERVER_CLOSED
    };

    /** Enable or disable the notifications from the enum above: */
    void menuNotificationsSetEnabled(bool enabled);

signals:
    /** Signal sent when a dock widget was opened: */
    void dockWidgetOpened(QDockWidget *widget);

    /**
     * Those 2 signals below are sent when the user wants to open
     * the JoinRoom or HostRoom dialog. They are good signals for
     * the buttons in the menu bar for example. The Menu class
     * might be the one capturing those signals.
     */
    void joinRoomMenuBtnClicked();
    void hostRoomMenuBtnClicked();

    /**
     * Signal sent when the user wants to leave the room.
     * The Room class might be the one capturing this signal.
     */
    void leaveRoomMenuBtnClicked();

    /**
     * Signal to be sent to the Room when a relevant
     * button is pressed in the menu bar. A button that
     * belongs to the room actions. The Room will
     * handle the actions. @param index is the index
     * of the button clicked (so the Room can
     * distinguish between buttons). Instead of using this
     * signal, it is safer to call the method
     * sendRoomButtonClickedSignal(index) to send this
     * signal for you.
     */
    void roomButtonClicked(int index);
    
protected slots:

    /* Open & switch views: */

    void openLoadingView(RoomController *controller);

    void openMainMenu(qint8 message = MSG_NO_MESSAGE);

    void openRoomView();

    void openSettingsDialog();

    /** Opens any dock widget. The signal dockWidgetOpened() is emitted after that. */
    void openDockWidget(QDockWidget *widget, Qt::DockWidgetArea area, bool isFloating = false);

    /** Puts one dock widget over the other: */
    void tabifyDock(QDockWidget *first, QDockWidget *second);

    /** Slot triggered by the Room asking to enable/disable a button: */
    virtual void buttonSetEnabled(int button, bool enabled);

protected:
    /*
     * This method can be overwritten outside the engine to implement
     * a different way to deal with notifications:
     */
    virtual void displayMenuNotification(ReturnMessages message);

    /* Called when the Main Menu was opened: */
    virtual void on_mainMenu_loaded() = 0;

    /* Called when the Loading Screen was opened: */
    virtual void on_loadingScreen_loaded() = 0;

    /* Called when the Room was opened: */
    virtual void on_roomView_loaded() = 0;

    /**
     * Call this function to emit the roomButtonClicked(int) signal.
     * This class will only emit this signal if the user is in the Room.
     * @return true if the signal was sent.
     */
    bool sendRoomButtonClickedSignal(int index);

    /* Getters for the views: */
    QWidget* getMainMenu();
    QWidget* getRoomView();

private:
    void setRoomController(RoomController *controller);
    void addChatBox();
    void removeChatBox();
    void removeDockWidgets();

    /*
     * Pointers to views or dialogs:
     */
    LoadingScreen *loadingScreen;
    QDockWidget *chatDockWidget;
    QWidget *settingsDialog;
    Menu *mainMenu;
    Room *roomView;

    /*
     * Pointer to the Room Controller
     * It always assumes a user can only be in one room at once.
     */
    RoomController *roomController;

    /*
     * Pointer to the User Settings.
     * It stores some personal settings of the user:
     */
    UserSettings *userSettings;

    /*
     * If menu notifications are enabled, the user can
     * receive messages in the main menu related to the
     * previous room he was in. Messages such as "You were
     * disconnected.
     */
    bool menuNotifications;
    
};

#endif // VIEWCONTROLLER_H
