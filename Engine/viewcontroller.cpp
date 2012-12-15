#include "viewcontroller.h"

ViewController::ViewController(QWidget *parent) :
    QMainWindow(parent)
{
    /* Make sure the Engine is running: */
    if (!Engine::isRunning()) {
        qCritical() << "[ERROR] Engine class called, but Engine was not started!";
        return;
    }

    /* By default, the user can see notifications: */
    menuNotifications = true;

    /*
     * Declare views and widgets that might be requested:
     */
    chatDockWidget = NULL;
    mainMenu = NULL;
    loadingScreen = NULL;
    roomView = NULL;

    /*
     * The View Controller does not (and must not) know anything about UserSettings.
     * However, it does initialize and stores a pointer to it. It is also
     * reponsible for deleting it.
     */
    userSettings = Engine::newUserSettings(this);

    /*
     * Alright, ViewController knows that UserSettings emits a signal asking to open the Settings Dialog:
     */
    connect(userSettings, SIGNAL(openSettingsDialog()), this, SLOT(openSettingsDialog()));

    /*
     * Initialize the SettingsDialog. This is a dialog that allows the user to change personal
     * settings. UserSettings is the one who stores it. The ViewController
     * doesn't know anything about this dialog. It just opens it when required.
     */
    settingsDialog = Engine::newSettingsDialog(userSettings, this);

    /*
     * RoomController is one of the main controllers of the application. ViewController must not
     * know anything about it. It does initialize/delete it anyway.
     */
    roomController = NULL;
}

ViewController::~ViewController()
{
    userSettings->deleteLater();
    setRoomController(NULL); //This will delete the room controller
}

void ViewController::menuNotificationsSetEnabled(bool enabled)
{
    menuNotifications = enabled;

    /* This is good to be used when testing the application
      to not get annoyed by the notifications. */
}

/**
 * This is how the View Controller stores a pointer to the RoomController.
 * There must be only one room controller because the user will only be
 * in one room at the same time. So, this class will delete the
 * previous controller and stores the new one. This happens when
 * the user changes room.
 */
void ViewController::setRoomController(RoomController *controller)
{
    if ( (roomController != NULL) && (roomController != controller) ) {
        qDebug() << "[ViewController] Previous Room Controller deleted.";
        delete roomController;
    }

    roomController = controller;
}

/**
 * The main menu should be opened as soon as the app starts.
 * When this function is called, the main menu is opened.
 */
void ViewController::openMainMenu(qint8 message)
{

    /* Make sure you are not already in the main menu: */
    if ( centralWidget() == dynamic_cast<QWidget*>(mainMenu) ) return;

    /* Hide the chat box (if there is one opened): */
    removeChatBox();

    /* Remove all dock widgets: */
    removeDockWidgets();

    /* Remove the Room View if is still opened (in case loading failed): */
    if (roomView) roomView->deleteLater();

    /*
     * A new MainMenu is declared:
     */
    mainMenu = Engine::newMainMenu(userSettings, this);

    /* The Main Menu has the power of closing the app entirely: */
    connect(mainMenu, SIGNAL(signalClose()), this, SLOT(close()));

    /*
     * From the Main Menu, the user can go to the loading screen (by joining or hosting a room),
     * or request to open the FindRoom or NewRoom dialog:
     */
    connect(mainMenu, SIGNAL(goToRoom(RoomController*)), this, SLOT(openLoadingView(RoomController*)));
    connect(this, SIGNAL(joinRoomMenuBtnClicked()), mainMenu, SLOT(openFindRoomDialog()));
    connect(this, SIGNAL(hostRoomMenuBtnClicked()), mainMenu, SLOT(openNewRoomDialog()));

    /*
     * The class that inherits this one will implement that function:
     */
    on_mainMenu_loaded();

    /*
     * Display the MainMenu inside the MainWindow, so that users can see it:
     */
    setCentralWidget(mainMenu);

    /*
     * Display a message depending on how the user got there:
     */
    displayMenuNotification((ReturnMessages) message);
}

void ViewController::displayMenuNotification(ReturnMessages message)
{
    if (menuNotifications) { /* This is what menuNotifications are for! */
        switch(message) {
        case MSG_BANNED:
            QSound::play("Sound/connectionFailed.wav");
            QMessageBox::information(this, tr("You are banned"),
                                     tr("The host banned you from the room. You are not allowed to come back."));
            break;

        case MSG_DISCONNECTED:
            QSound::play("Sound/connectionFailed.wav");
            QMessageBox::information(this, tr("You got disconnected"),
                                     tr("For some reason, you got disconnected from the room. Please, try again later."));
            break;

        case MSG_SERVER_CLOSED:
            QSound::play("Sound/connectionFailed.wav");
            QMessageBox::information(this, tr("The server is now closed"),
                                     tr("The server has been closed."));
            break;

        case MSG_DROPPED:
            QSound::play("Sound/connectionFailed.wav");
            QMessageBox::information(this, tr("You got disconnected"),
                                     tr("For some reason, the host dropped you from the room. Please, try again later."));
            break;

        case MSG_NO_MESSAGE:

            break;
        }
    }
}

bool ViewController::sendRoomButtonClickedSignal(int index)
{
    /* Signal can only be sent from inside the room: */
    if (centralWidget() != getRoomView()) return false;

    emit roomButtonClicked(index);
    return true;
}


/**
  Go to the loading screen.
  This function will be called by the MainMenu when a user joins or hosts a room:
  */
void ViewController::openLoadingView(RoomController *controller)
{
    if (controller == NULL) {
        qCritical() << "[ViewController] You tried to go to an invalid room. Operation aborted.";
        return;
    }

    /* And we set the RoomController without spying on what is inside: */
    setRoomController(controller);

    /* Declare a new instance of the RoomView, so that the LoadingView can handle it: */
    roomView = Engine::newRoomView(this);

    /*
     * Add the Room Controller to the Room View:
     * This is not done in the constructor to hide the Room Controller from the
     * class that inherits RoomView outside the Engine.
     */
    roomView->setRoomController(controller);

    /* Declare a new instance of the LoadingView: */
    loadingScreen = Engine::newLoadingScreen(roomView, this);

    /* Add the Room Controller to the loading screen: */
    loadingScreen->setRoomController(controller);

    /*
     * The class that inherits this one will implement that function:
     */
    on_loadingScreen_loaded();

    /*
     * The loading view has the power to go back to the main menu (eg. loading failed)
     * or go to the Room View where all the action happens:
     */
    connect(loadingScreen, SIGNAL(goBackToMenu()), this, SLOT(openMainMenu()), Qt::QueuedConnection);
    connect(loadingScreen, SIGNAL(openRoomView()), this, SLOT(openRoomView()), Qt::QueuedConnection);

    /* Display the Loading Screen to the user: */
    setCentralWidget( loadingScreen );

    /* Ask the loading screen to start loading the room: */
    loadingScreen->startLoading();
}

/**
 * Go to the Room view.
 * So far, the only way to get there is through the loading screen:
 */
void ViewController::openRoomView()
{
    if (roomView == NULL) {
        qWarning() << "[ViewController] Cannot open the Room View because the view is NULL.";
        return;
    }

    /*
     * The user can leave the room through a button. So,
     * ViewController must send a signal to the room asking to leave the room.
     * This job ViewController cannot do because there are connections to be closed.
     * ViewController knows nothing about connections because they are in the
     * RoomController.
     */
    connect(this, SIGNAL(leaveRoomMenuBtnClicked()), roomView, SLOT(leaveRoom()));

    /* Allow buttons in the menu bar to warn the room view when they are clicked: */
    connect(this, SIGNAL(roomButtonClicked(int)), roomView, SLOT(roomButtonClicked(int)));

    /* Allow the room to enable/disable the buttons in the menu bar: */
    connect(roomView, SIGNAL(buttonSetEnabled(int,bool)), this, SLOT(buttonSetEnabled(int,bool)));

    /*
     * The Room View can open docks in case it needs:
     */
    connect(roomView, SIGNAL(openDockWidget(QDockWidget*,Qt::DockWidgetArea, bool)),
               this, SLOT  (openDockWidget(QDockWidget*,Qt::DockWidgetArea, bool)));

    /* The Room View can also tabify dock widgets: */
    connect(roomView, SIGNAL(tabifyDockWidget(QDockWidget*,QDockWidget*)),
               this, SLOT  (tabifyDock(QDockWidget*,QDockWidget*)));

    /*
     * The room might want a confirmation that the dock widget was opened:
     */
    connect(this, SIGNAL(dockWidgetOpened(QDockWidget*)),
         roomView, SLOT  (dockWidgetOpened(QDockWidget*)));


    /*
     * Once the Room View is about to be opened, we can add the chat box to the view:
     */
    addChatBox();

    /*
     * The class that inherits this one will implement that function:
     */
    on_roomView_loaded();

    /*
     * The Room View is the main action of the app. But its powers here are limited
     * to go back to the main Menu.
     */
    connect(roomView, SIGNAL(goBackToMenu(qint8)), this, SLOT(openMainMenu(qint8)), Qt::QueuedConnection);

    /* Display the Room Screen to the user: */
    setCentralWidget( roomView );

    /* Try to start a room: */
    if (!roomView->startRoom()) return;
}

/**
 * Add a chat box to the screen, so users can talk to each other.
 * It is assuming that there is already a connection established:
 */
void ViewController::addChatBox()
{
    /* The Chat is inside a DockWidget (a window that can be attached to the MainWindow): */
    chatDockWidget = new QDockWidget(this);

    /* You can put the chat anywhere in the corners of the main window: */
    chatDockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);

    /* You can move the chat around and detach it from the main window: */
    chatDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    /*
     * Initializes the chatBox. It needs to know about the chatDockWidget to be able to
     * change the title of the dock:
     */
    ChatRoom *chatBox = new ChatRoom(roomController, chatDockWidget);

    /*
     * The chat box can open private chat boxes that are in different dock widgets.
     * So, allow it to do so dynamically:
     */
    connect(chatBox, SIGNAL(openDockWidget(QDockWidget*,Qt::DockWidgetArea, bool)),
               this, SLOT  (openDockWidget(QDockWidget*,Qt::DockWidgetArea, bool)));

    /* The chat box can also tabify dock widgets: */
    connect(chatBox, SIGNAL(tabifyDockWidget(QDockWidget*,QDockWidget*)),
               this, SLOT  (tabifyDock(QDockWidget*,QDockWidget*)));

    /*
     * The chat box might want a confirmation that the dock widget was opened:
     */
    connect(this, SIGNAL(dockWidgetOpened(QDockWidget*)),
         chatBox, SLOT  (dockWidgetOpened(QDockWidget*)));

    /* Add the chatBox inside the chat dock: */
    chatDockWidget->setWidget( chatBox );

    /* Add the chat dock to the bottom of the screen: */
    addDockWidget((Qt::DockWidgetArea) CHAT_BOX_AREA, chatDockWidget);

}

/**
 * When the user leaves the Room View, the chat box will be removed and deleted:
 */
void ViewController::removeChatBox()
{
    if (chatDockWidget == NULL) return;
    chatDockWidget->close();
    delete chatDockWidget;
    chatDockWidget = NULL;
}

/** Removes all dock widgets from the view: */
void ViewController::removeDockWidgets()
{
    QList<QDockWidget *> dockWidgets = findChildren<QDockWidget *>();
    foreach (QDockWidget* dock, dockWidgets)
        removeDockWidget(dock);
}

QWidget* ViewController::getMainMenu()
{
    return mainMenu;
}

QWidget *ViewController::getRoomView()
{
    return roomView;
}

/**
 * Opens a dock widget.
 * A different view class connected to the main window can signal
 * the view controller to display a dock widget:
 */
void ViewController::openDockWidget(QDockWidget *widget, Qt::DockWidgetArea area, bool isFloating)
{
    if (widget == NULL) {
        qWarning() << "[ViewController] Tried to add a NULL widget to the screen.";
        return;
    }
    addDockWidget(area, widget);
    widget->setFloating(isFloating); //A floting dock behaves like a different window.
    widget->activateWindow();
    emit dockWidgetOpened(widget);
}

/** Put one dock widget together with the other: */
void ViewController::tabifyDock(QDockWidget *first, QDockWidget *second)
{
    tabifyDockWidget(first, second);
    second->setVisible(true);
    second->setFocus();
    second->raise();
}

/** Can be implemented by the class that inherits this one to enable/disable buttons: */
void ViewController::buttonSetEnabled(int button, bool enabled) {
    Q_UNUSED(enabled);
    qDebug() << "[ViewController] There are no buttons defined to enable/disable button " << button
             << ". You need to reimplement this method in the class that implements ViewController.";
}

/**
 * The settings dialog allow the user to change personal settings:
 */
void ViewController::openSettingsDialog()
{
    settingsDialog->show();
    settingsDialog->activateWindow();
}
