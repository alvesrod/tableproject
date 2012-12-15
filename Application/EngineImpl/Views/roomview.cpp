#include "roomview.h"
#include "ui_roomview.h"

RoomView::RoomView(QWidget *parent) :
    Room(parent),
    ui(new Ui::RoomView)
{
    actionController = NULL;
    tableController = NULL;
    dialogController = NULL;
    member = NULL;
    roomInfo = NULL;


    /*
     * Constructor is called together with the loading screen. So,
     * this should not be heavy. All the heavy data should be done
     * in the on_loadingScreen_load() method.
     */

}

RoomView::~RoomView() //Destructor
{
    if (!isRoomLoaded())
        /*
         * "if" because the room view becomes the parent of the table when
         * the room is loaded. The table is added to the view after that,
         * which means it would be automatically deleted.
         */
        dynamic_cast<RoomInfo*> (getRoomDescription())->getTable()->deleteLater();
    delete ui;
}

/**
    This function is called by the loading screen asking to prepare room BEFORE data has arrived.
    Remember that this function is called before knowing whether or not the user will be allowed
    to join the room. So, be careful because if the user can't join the room, this class will be
    deleted right after.
    @return true if the room was successfully prepared.
*/
bool RoomView::on_loadingScreen_prepare()
{
    /*
     * Get all the info about the room. Typecast from RoomDescription* to RoomInfo*. RoomInfo
     * is a class outside the Engine that extends RoomDescription:
     */
    roomInfo = dynamic_cast<RoomInfo*> (getRoomDescription());

    return true;
}

/**
    This function is called by the loading screen asking to load room AFTER data has arrived.
    This is where all the heavy loading is done. It can take a while here loading data.
    Also, at this point you are not inside the Room yet. So, you can't have your User object
    yet, and if you request the list of users in the room, you won't be there.
    @return true if the room was successfully loaded.
*/
bool RoomView::on_loadingScreen_load()
{
    ui->setupUi(this);

    tableController = new TableController(this);

    /* Connect signals and slots for the table controller: */
    tableControllerMakeConnections();

    actionController = new ActionController(this);

    /* Connect signals and slots in the action controller: */
    actionControllerMakeConnections();

    /* Adjust the size of the table based on the size of the window: */
    ui->roomSpacer->changeSize(0, 0.67 * parentWidget()->height());

    return true;
}

/**
    This function is called by the mainMenu when the room is loaded.
    You can assume from here that the user is already inside the room
    and all the connections to send network messages are set. The
    computation of this class is still done inside the loading screen,
    but the room view is already loaded.
    @return true if the room started well.
*/
bool RoomView::on_room_loaded()
{
    /*
     * Get the user (you). Typecast from User* to RoomMember*. RoomMember
     * is a class outside the Engine that extends User:
     */
    member = dynamic_cast<RoomMember*> (getCurrentUser());

    if (member == NULL) {
        qWarning() << "[RoomView] ERROR: Starting room with a null member! This shouldn't happen!";
        return false;
    }

    /* Make connections to signals emmited by the user: */
    userMakeConnections();

    dialogController = new DialogController(this);

    /* Connect signals and slots for the dialog controller: */
    dialogControllerMakeConnections();

    /* Load the table for member: */
    table()->load();

    /* Remove item ownership of all users that are not in the room: */
    table()->fixItemOwnerships(users());

    /* Set up what you can do based on permissions you might have: */
    updatePermissions();

    /*
     * Add the table to the view, so users can see it.
     * This has to be done after updating permissions to avoid a hidden item to blink
     * the image in a slow computer:
     */
    ui->roomLayout->addWidget( table() );

    /* Say hello to the new user! */
    displayWelcomeChatMessage();

    /* Start timer to save the table periodically: */
    startAutosaveTimer();

    /* Play a song to the host: */
    if (member->isTheHost())
        QSound::play("Sound/hostRoom.wav");
    else
        QSound::play("Sound/joinRoom.wav");

    /* Remove the room spacer to allow resizing now that the size was adjusted: */
    QTimer::singleShot(1, this, SLOT(allowResizing()));

    return true;
}

/**
    Called when a room is about to close.
    @return false if you must not close the room.
  */
bool RoomView::on_leaving_room()
{
    /* The user is leaving! Do whatever you have to do before he/she leaves! */
    autosave.stop();
    info()->autosaveTableFile();
    QSound::play("Sound/leaveRoom.wav");
    return true;
}

/** Called when somebody joined the room: */
void RoomView::on_someone_joined_room(User *user)
{
    /* Give this person a new team based on the room settings: */
    assignTeam( dynamic_cast<RoomMember*>(user) );

    if (!user->isTheHost())
        QSound::play("Sound/newUser.wav");
}

/** Called when somebody leaves the room: */
void RoomView::on_someone_left_room(User *user)
{
    /* Make this user not the owner anymore of the table items: */
    table()->removeUserOwnership( dynamic_cast<RoomMember*>(user) );

    if (!user->isTheHost())
        QSound::play("Sound/userLeft.wav");
}

/** Called when you receive a message from the server: */
void RoomView::actionMessageReceived(QByteArray message, User *user, qint8 type)
{
    /* The action controller will handle action messages: */
    action()->actionMessageReceived(message, dynamic_cast<RoomMember*>(user), type);
}

/** Called when you receive a random value from the server: */
void RoomView::randomValuesReceived(qint32 min, qint32 max, qint16 type, QList<qint32> values, User *user)
{
    /* The action controller will handle the random values: */
    action()->randomValuesReceived(min, max, type, values, dynamic_cast<RoomMember*>(user));
}

void RoomView::large_message_finished(qint8 type, QByteArray info, QByteArray package, User *user)
{
    /* The action controller will handle large messages: */
    action()->large_message_finished(type, info, package, dynamic_cast<RoomMember*>(user));
}

void RoomView::large_message_progress(qint8 type, QByteArray info, double percentage, User *user)
{
    /* The action controller will handle large messages: */
    action()->large_message_progress(type, info, percentage, dynamic_cast<RoomMember*>(user));
}

void RoomView::large_message_cancelled(qint8 type, QByteArray info, User *user)
{
    /* The action controller will handle large messages: */
    action()->large_message_cancelled(type, info, dynamic_cast<RoomMember*>(user));
}

void RoomView::large_message_started(qint8 type, QByteArray info, qint32 size, User *user)
{
    /* The action controller will handle large messages: */
    action()->large_message_started(type, info, size, dynamic_cast<RoomMember*>(user));
}

/** Deal with the buttons in the menu bar when they are pressed: */
void RoomView::roomButtonClicked(int index)
{
    switch (index) {

    case BTN_THROW_DICE:
        dialogController->openThrowDiceDialog();
        break;

    case BTN_ADD_OBJECT:
        hideWelcomeText();
        dialogController->openAddObjectDialog();
        break;

    case BTN_ZOOM_IN:
        table()->zoomTable(ZOOM_TABLE_PORCENTAGE);
        break;

    case BTN_ZOOM_OUT:
        table()->zoomTable(-ZOOM_TABLE_PORCENTAGE);
        break;

    case BTN_ROTATE_LEFT:
        table()->rotateTable(-ROTATE_TABLE_ANGLE);
        break;

    case BTN_ROTATE_RIGHT:
        table()->rotateTable(ROTATE_TABLE_ANGLE);
        break;

    case BTN_SAVE_TABLE:
        info()->saveAsTableFile(this); //This will open a dialog and save it
        break;

    case BTN_ROOM_INFO:
        dialogController->openRoomInfoDialog();
        break;

    default:
        qWarning() << "[RoomView] Warning: Received a button click from an unknown index: " << index;
    }
}

/** Connect signals and slots for the table: */
void RoomView::tableControllerMakeConnections()
{
    /* Allow the table to send messages: */
    connect(tableController, SIGNAL(sendActionMessage(qint8,QByteArray,User*,bool)),
            this, SLOT(sendActionMessage(qint8,QByteArray,User*,bool)), Qt::QueuedConnection);
    connect(tableController, SIGNAL(sendActionMessageToHost(qint8,QByteArray,bool)),
            this, SLOT(sendActionMessageToHost(qint8,QByteArray,bool)), Qt::QueuedConnection);
    connect(tableController, SIGNAL(sendLargeMessage(qint8,QByteArray,QByteArray,User*)),
            this, SLOT(sendLargeMessage(qint8,QByteArray,QByteArray,User*)), Qt::QueuedConnection);
    connect(tableController, SIGNAL(sendFile(qint8,QString,QByteArray,User*)),
            this, SLOT(sendFile(qint8,QString,QByteArray,User*)), Qt::QueuedConnection);

    /* Allow the Table Controller to write on the chat: */
    connect(tableController, SIGNAL(addChatMessage(QString,QColor)),
            this, SLOT(addChatMessage(QString,QColor)), Qt::QueuedConnection);
}

/** Connect signals and slots for the action controller: */
void RoomView::actionControllerMakeConnections()
{
    /* Allow the Action Controller to send messages: */
    connect(action(), SIGNAL(sendActionMessage(qint8,QByteArray,User*,bool)),
            this, SLOT(sendActionMessage(qint8,QByteArray,User*,bool)), Qt::QueuedConnection);
    connect(action(), SIGNAL(sendActionMessageToHost(qint8,QByteArray,bool)),
            this, SLOT(sendActionMessageToHost(qint8,QByteArray,bool)), Qt::QueuedConnection);
    connect(action(), SIGNAL(sendLargeMessage(qint8,QByteArray,QByteArray,User*)),
            this, SLOT(sendLargeMessage(qint8,QByteArray,QByteArray,User*)), Qt::QueuedConnection);
    connect(action(), SIGNAL(requestRandomValues(qint32,qint32,qint16,qint32,User*,bool)),
            this, SLOT(requestRandomValues(qint32,qint32,qint16,qint32,User*,bool)), Qt::QueuedConnection);
    connect(action(), SIGNAL(sendFile(qint8,QString,QByteArray,User*)),
            this, SLOT(sendFile(qint8,QString,QByteArray,User*)), Qt::QueuedConnection);

    /* Allow the Action Controller to write on the chat: */
    connect(action(), SIGNAL(addChatMessage(QString,QColor)),
            this, SLOT(addChatMessage(QString,QColor)), Qt::QueuedConnection);
}

/** Connect signals and slots for the dialog controller: */
void RoomView::dialogControllerMakeConnections()
{
    connect(dialogController, SIGNAL(lockItem(TableItem*, QList<qint8>)),
            tableC(), SLOT(lockItem(TableItem*, QList<qint8>)));
    connect(dialogController, SIGNAL(hideItem(TableItem*, QList<qint8>)),
            tableC(), SLOT(hideItem(TableItem*, QList<qint8>)));
    connect(tableC(), SIGNAL(requestLockItemDialog(TableItem*)),
            dialogController, SLOT(openLockItemDialog(TableItem*)));
    connect(tableC(), SIGNAL(requestHideItemDialog(TableItem*)),
            dialogController, SLOT(openHideItemDialog(TableItem*)));
}

/** Connect signals and slots for the user: */
void RoomView::userMakeConnections()
{
    connect(user(), SIGNAL(teamChanged(RoomMember*,RoomMember*)),
            this, SLOT(onTeamChanged(RoomMember*,RoomMember*)));
    connect(user(), SIGNAL(adminStatusChanged(User*,User*)),
            this, SLOT(onAdminStatusChanged(User*,User*)));
}

/** Display a message to who joins the room: */
void RoomView::displayWelcomeChatMessage()
{
    /* Hide welcome label if it's not the beggining of a room: */
    if ( (table()->totalOfItems() > 0) || (!member->isTheHost())
         || (!Permission::canAddObjects(member, info())) ) {
        hideWelcomeText();
    }

    addChatMessage("</i><font color='#49211D'>" + tr("You joined") /* You joined [ROOM] */
                    + " <font color='#375324'>\""
                   + info()->getRoomName() + "\" (" + info()->getRoomType()
                   + ").</font></font>");
    if (!info()->getRoomDescription().isEmpty())
        addChatMessage("</i><font color='#49211D'>" + tr("Info:") + " <font color='#375324'>"
                   + info()->getRoomDescription() + "</font></font>");
}

/** Start timer to save the table periodically: */
void RoomView::startAutosaveTimer()
{
    connect(&autosave, SIGNAL(timeout()), this, SLOT(autosaveTableFile()));
    int milliseconds = 1000 * AUTO_SAVE_TIME_SECONDS;
    autosave.start( milliseconds );
}

void RoomView::autosaveTableFile() {
    info()->autosaveTableFile();
}


/** When your team changed: */
void RoomView::onTeamChanged(RoomMember *memberChanged, RoomMember *memberWhoChanged)
{
    Q_UNUSED(memberChanged); Q_UNUSED(memberWhoChanged);

    updatePermissions();
}

/** When your admin status changed: */
void RoomView::onAdminStatusChanged(User *memberChanged, User *memberWhoChanged)
{
    Q_UNUSED(memberChanged); Q_UNUSED(memberWhoChanged);

    updatePermissions();
}

/** Remove the room spacer to allow resizing now that the size was adjusted: */
void RoomView::allowResizing()
{
    ui->roomSpacer->changeSize(0, 255);
}

/** Update buttons and others based on your permissions: */
void RoomView::updatePermissions()
{
    /* Disable the "Add Object" button if the user can't add an object: */
    emit buttonSetEnabled(BTN_ADD_OBJECT, Permission::canAddObjects(user(), info()));

    /* Let the table controller handle the table permissions: */
    tableC()->updatePermissions();
}

void RoomView::assignTeam(RoomMember *user)
{
    /* Let only the room host to assign a new team: */
    if (!getCurrentUser()->isTheHost()) return;

    switch(info()->getTeamSortingType()) {

    case RoomInfo::TEAM_ONE_PER_TEAM:

        /* Search for empty teams: */
        user->assignToEmptyTeam( users() );
        break;

    case RoomInfo::TEAM_RANDOM_TEAMS:

        /* Generate a random team for the user: */
        user->setTeam( (RoomMember::TeamColor) (qrand() % (RoomMember::TOTAL_OF_TEAMS)) );
        break;

    case RoomInfo::TEAM_SAME_TEAM:

        /*
         * No need to change team here since they are in the same team.
         * But, you still have to send the message because the way the
         * Room member class was done, it is expecting an initial team.
         */
        user->setTeam( RoomMember::TEAM_OTHERS );
        break;

    default:
        qWarning() << "[RoomView] Inappropriate type: " << info()->getTeamSortingType();
    }

    /* Send message warning others that the user change team: */
    user->sendEditUserMessage(RoomMember::MSG_CHANGE_TEAM);
}

/** Hides the welcome text from the beggining: */
void RoomView::hideWelcomeText()
{
      ui->welcomeText->hide();
}

/** Retranslates the app if its language changed: */
void RoomView::changeEvent(QEvent* event)
{
    /* No need to retranslate components if user is not in the room. */
    if ( (event->type() == QEvent::LanguageChange) && (userInRoom()) )
        ui->retranslateUi(this);
    QWidget::changeEvent(event);
}

/* Getters for the main components: */
RoomMember *RoomView::user() { return member; }
RoomMember *RoomView::user(qint32 userID) { return dynamic_cast<RoomMember*>(getUser(userID)); }
RoomMember *RoomView::hostUser() { return dynamic_cast<RoomMember*>(getHostUser()); }
ActionController *RoomView::action() { return actionController; }
TableController *RoomView::tableC() { return tableController; }
RoomInfo *RoomView::info() { return roomInfo; }
Table *RoomView::table() { return info()->getTable(); }
QList<User *> RoomView::users() { return getUsersInRoom(); }

