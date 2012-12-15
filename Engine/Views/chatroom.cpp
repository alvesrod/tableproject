#include "chatroom.h"
#include "ui_chatroom.h"

ChatRoom::ChatRoom(RoomController *controller, QWidget *parent):
    QWidget(parent),
    ui(new Ui::ChatRoom)
{
    /* Make sure the Engine is running: */
    if (!Engine::isRunning()) {
        qCritical() << "[ERROR] Engine class called, but Engine was not started!";
        return;
    }

    if (controller == NULL) {
        qWarning() << "[ChatBox] Error: Room Controller is null.";
        return;
    }

    ui->setupUi(this);

    roomController = controller;
    roomController->setChatroom(this);
    totalUsers = 0;
    latestUserBox = NULL;
    serverMessageColor = SERVER_MESSAGE_COLOR;
    actionLog = NULL;

    declareConnections();
    setUpChatroomUI();

}

ChatRoom::~ChatRoom()
{
    foreach (QDockWidget* widget, privateChats)
        widget->deleteLater();
    foreach (QDockWidget* widget, userBoxes)
        widget->deleteLater();
    privateChats.clear();
    userBoxes.clear();

    /* Make sure the roomController doesn't have access to a dangling pointer: */
    roomController->setChatroom(NULL);
}

void ChatRoom::declareConnections()
{
    connect( roomController, SIGNAL(chatMessageReceived(QByteArray, User*)), this, SLOT(chatMessageReceived(QByteArray, User*)));
    connect( roomController, SIGNAL(someoneConnectedToServer(User*)), this, SLOT(someoneConnected(User*)));
    connect( roomController, SIGNAL(someoneDisconnectedFromServer(User*)), this, SLOT(someoneDisconnected(User*)));
    connect( roomController, SIGNAL(privateChatMessageReceived(QByteArray,User*)),
                       this, SLOT  (privateChatMessageReceived(QByteArray,User*)));
    connect( roomController->getUserSettings(), SIGNAL(chatFontChanged(QFont)), this, SLOT(chatFontChanged(QFont)));
}

/** Add a message to the chat (only for you): */
void ChatRoom::addChatMessage(QString message, User *user, QColor defaultColor)
{
    QString color = "";
    if (defaultColor.isValid()) color = defaultColor.name();
    appendChatMessage( getStringMessage(message, user, color) );
}

void ChatRoom::addLog(QListWidgetItem *logItem)
{
    if (actionLog == NULL) setNewActionLogWidget();
    actionLog->actionLog(logItem);
    actionLog->show();
    logBox->show();
}

void ChatRoom::logText(QString text, QColor color)
{
    QListWidgetItem *item = new QListWidgetItem(text);
    item->setBackgroundColor(QColor(223,225,187));
    item->setTextColor(color);
    addLog(item);
}

void ChatRoom::setUpChatroomUI()
{
    linkViewComponents(chatText, chatLineEdit, usersBox);
    addExistingUsersToChat();
    chatLineEdit->setFocus();
    chatText->setFont( roomController->getUserSettings()->getChatFont() );
}

void ChatRoom::privateChatMessageReceived(QByteArray message, User *user)
{
    PrivateChatBox *pChat = getPrivateMessageWidget(user, false);
    if (pChat == NULL) {
        qWarning() << "Received a message, but the chat box is NULL.";
        return;
    }

    QSound::play("Sound/privateChatBeep.wav");

    /* Add the message inside the private chat box: */
    pChat->addChatLine( getStringMessage(QString::fromUtf8(message), user) );
}

void ChatRoom::chatMessageReceived(QByteArray message, User *user)
{
    if (!user->isThatYou())
        QSound::play("Sound/chatMessage.wav");
    addChatMessage(QString::fromUtf8(message), user);
}

void ChatRoom::privateMessageToBeSent(QString message, User *to)
{
    roomController->sendMessageToServer( RoomController::MSG_PRIVATE_CHAT_MESSAGE , message.toUtf8(), to );

     /* Show the message to yourself too: */
    PrivateChatBox *pChat = getPrivateMessageWidget(to);
    pChat->addChatLine( getStringMessage(message, roomController->getCurrentUser()) );
}

void ChatRoom::updateName(User *user)
{
    setUserBoxUsername(user);

    /* Update the name in the private chat window: */
    if ( (privateChats.contains(user)) && (privateChats.value(user) != NULL) ) {
        QDockWidget *privateChat = privateChats.value(user);
        privateChat->setWindowTitle(getPrivateChatTitle(user));
        PrivateChatBox *box = dynamic_cast<PrivateChatBox*> (privateChat->widget());
        box->setUsername();
    }

    /* Update the name in the user box window: */
    if ( (userBoxes.contains(user)) && (userBoxes.value(user) != NULL) ) {
        QDockWidget *userBox = userBoxes.value(user);
        userBox->setWindowTitle(getUserBoxTitle(user));
    }
}

void ChatRoom::setUserBoxUsername(User *user)
{
    int boxIndex = getUserBoxIndex(user);
    if (boxIndex < 0) {
        qWarning() << "[Chat] Tried to change userbox username, but user is not in the room.";
        return;
    }
    userBoxChangeUsernameTitle( boxIndex, prepareUsernameForUserBox(user) );
}

int ChatRoom::getTotalUsersInChat()
{
    return totalUsers;
}

void ChatRoom::setServerMessageColor(QString color)
{
    serverMessageColor = color;
}

QString ChatRoom::prepareUsernameForUserBox(User *user)
{
    QString nickname = user->getNickname();
    if (user->isThatYou()) {
        nickname.append(" " + tr("[you]"));
    }
    return nickname;
}

/** String message used for the chat where it displays the user name and/or sets the color: */
QString ChatRoom::getStringMessage(QString body, User *user, QString defaultColor)
{
    QString head = getMessageHead(user, defaultColor);
    return QString(head + body);
}

QString ChatRoom::getMessageHead(User *user, QString defaultColor)
{
    QString name = "";
    QString color = "<font color='";
    if (user == NULL) {
        name.append("<i>");
        if (defaultColor.isEmpty())
            defaultColor = serverMessageColor;
        color.append(defaultColor);
    } else {
        name.append("<b>");
        name.append(user->getNickname() + ": </b>");
        color.append(user->getMemberChatColor().name());
    }
    color.append("'>");
    return (name + color);
}

PrivateChatBox *ChatRoom::getPrivateMessageWidget(User *user, bool localRequest)
{
    if (user == NULL) return NULL;

    QDockWidget *privateChat;
    if ( (privateChats.contains(user)) && (privateChats.value(user) != NULL) ) {
        privateChat = privateChats.value(user);
        if ( (!localRequest) && (!privateChat->isVisible()))
            QSound::play("Sound/privateChat.wav");
        privateChat->show();
        privateChat->activateWindow();
        PrivateChatBox *box = dynamic_cast<PrivateChatBox*> (privateChat->widget());
        box->focusToType();
    } else {
        privateChat = setNewPrivateMessageWidget(user);
        if (!localRequest)
            QSound::play("Sound/privateChat.wav");
    }
    return dynamic_cast<PrivateChatBox*> (privateChat->widget());
}

UserBox *ChatRoom::getUserBoxWidget(User *user)
{
    if (user == NULL) return NULL;

    QDockWidget *userBox;
    if ( (userBoxes.contains(user)) && (userBoxes.value(user) != NULL) ) {
        userBox = userBoxes.value(user);
        if (!userBox->isVisible())
            QSound::play("Sound/menuClick.wav");
        userBox->show();
        userBox->activateWindow();
        userBox->raise();
    } else {
        userBox = setNewUserBoxWidget(user);
    }
    return dynamic_cast<UserBox*> (userBox->widget());
}

QDockWidget* ChatRoom::setNewPrivateMessageWidget(User *user)
{
    QDockWidget *privateChat;
    privateChat = new QDockWidget( getPrivateChatTitle(user) , this);
    privateChat->setAllowedAreas(Qt::AllDockWidgetAreas);
    privateChat->setFeatures(QDockWidget::AllDockWidgetFeatures);
    PrivateChatBox *box = new PrivateChatBox(privateChat, user);
    box->setChatFont( roomController->getUserSettings()->getChatFont() );
    privateChat->setWidget( box );
    privateChats[user] = privateChat;
    privateChat->resize(PRIVATE_CHAT_BOX_WIDTH,PRIVATE_CHAT_BOX_HEIGHT);

    connect(box, SIGNAL(privateMessageToBeSent(QString,User*)),
                      this,   SLOT(privateMessageToBeSent(QString,User*)));

    emit openDockWidget(privateChat, Qt::BottomDockWidgetArea, true);
    return privateChat;
}

QDockWidget *ChatRoom::setNewUserBoxWidget(User *user)
{
    QDockWidget *userBox;
    userBox = new QDockWidget(getUserBoxTitle(user) , this);
    userBox->setAllowedAreas(Qt::AllDockWidgetAreas);
    userBox->setFeatures(QDockWidget::AllDockWidgetFeatures);
    UserBox *box = Engine::newUserBox(user, this);
    box->setRoomController(roomController);
    userBox->setWidget( box );
    userBoxes[user] = userBox;
    QSound::play("Sound/menuClick.wav");

    emit openDockWidget(userBox, Qt::BottomDockWidgetArea, false);
    return userBox;
}

/** Refresh the title of the main chat: */
void ChatRoom::refreshTitle()
{
    QString title(" " + tr("General Chat"));
    if (totalUsers != 1)
        title.append( QString(": ") + QString::number(totalUsers) + " " + tr("users") );
    parentWidget()->setWindowTitle(title);
}

/** Refresh the title of the action log: */
void ChatRoom::refreshActionLogTitle()
{
    if (logBox == NULL) return;
    logBox->setWindowTitle(" " + tr("Action Log"));
}

void ChatRoom::setNewActionLogWidget()
{
    QDockWidget *actionLogBox;
    actionLogBox = new QDockWidget(" ", this);
    actionLogBox->setAllowedAreas(Qt::AllDockWidgetAreas);
    actionLogBox->setFeatures(QDockWidget::AllDockWidgetFeatures);
    ActionLog *log = new ActionLog(this);
    connect( log, SIGNAL(selected(QModelIndex)), this, SLOT(logSelected(QModelIndex)));
    actionLogBox->setWidget( log );
    actionLog = log;
    logBox = actionLogBox;
    refreshActionLogTitle();
    emit openDockWidget(actionLogBox, Qt::BottomDockWidgetArea, false);
}

void ChatRoom::addExistingUsersToChat()
{
    if (roomController->getListOfUsers().isEmpty()) {
       // qDebug() << "[Chat] List of users is empty. Operation aborted";
        return;
    }

    foreach (User *user, roomController->getListOfUsers())
        addUserToChat(user);

    refreshTitle();
}

void ChatRoom::addUserToChat(User *user)
{
    UserButtonBox *details = new UserButtonBox(user, roomController->getCurrentUser(), roomController->getRoomDescription(), this);
    connect(details, SIGNAL(privateMsgBtnClicked(User*)), SLOT(privateMessageBtnClicked(User*)));
    connect(details, SIGNAL(moreBtnClicked(User*)), SLOT(moreBtnClicked(User*)));
    connect(details, SIGNAL(banUserBtnClicked(User*)), SLOT(banBtnClicked(User*)));

    connect(user, SIGNAL(nicknameChanged(User*,User*,QString)), this, SLOT(nicknameUpdateFromUser(User*,User*,QString)));
    connect(user, SIGNAL(adminStatusChanged(User*,User*)), this, SLOT(adminStatusUpdateFromUser(User*,User*)));

    addUserToUserBox(user, details, prepareUsernameForUserBox(user) );
    totalUsers++;
}

QString ChatRoom::getPrivateChatTitle(User *user)
{
    return tr("Chat:") + " " + user->getNickname();
}

QString ChatRoom::getUserBoxTitle(User *user)
{
    return tr("Info:") + " " + user->getNickname();
}

void ChatRoom::someoneConnected(User *user)
{
    addChatMessage(user->getNickname() + " " + tr("joined the room."));
    addUserToChat(user);
    refreshTitle();
}

void ChatRoom::someoneDisconnected(User *user)
{
    addChatMessage( getUserDisconnectedMessage(user) );

    int index = getUserBoxIndex(user);
    if (index == -1) {
        qDebug() << "[Chat] Someone disconnected, but could not be found on the chat box.";
        return;
    }
    removeUserFromUserBox(user, index);
    totalUsers--;
    refreshTitle();

    //Disable the private chat with the user (if there's one opened):
    if ( (privateChats.contains(user)) && (privateChats.value(user) != NULL) ) {
        QDockWidget *privateChat = privateChats.value(user);
        PrivateChatBox *box = dynamic_cast<PrivateChatBox*> (privateChat->widget());
        box->disableChat();
        box->addChatLine( getUserDisconnectedMessage(user) );
    }

    //Disable the user box (if there's one opened):
    if ( (userBoxes.contains(user)) && (userBoxes.value(user) != NULL) ) {
        QDockWidget *userBox = userBoxes.value(user);
        UserBox *box = dynamic_cast<UserBox*> (userBox->widget());
        box->userLeft();
    }
}

QString ChatRoom::getUserDisconnectedMessage(User *user)
{
    return QString(user->getNickname() + " " + tr("left the room."));
}

void ChatRoom::chatFontChanged(QFont font)
{
    setChatFont(font);
    foreach (QDockWidget* privateChat, privateChats) {
        PrivateChatBox *box = dynamic_cast<PrivateChatBox*> (privateChat->widget());
        box->setChatFont(font);
    }
}

void ChatRoom::privateMessageBtnClicked(User *user)
{
    getPrivateMessageWidget(user);
}

void ChatRoom::moreBtnClicked(User *user)
{
    getUserBoxWidget(user);
}

void ChatRoom::banBtnClicked(User *user)
{
    roomController->sendMessageToServer( RoomController::MSG_BAN_USER, 0, user);
}

void ChatRoom::logSelected(QModelIndex index)
{
    emit actionLogSelected(index);
}

void ChatRoom::adminStatusUpdateFromUser(User *memberChanged, User *memberWhoChanged)
{
    QColor color = QColor(78,69,101);
    if (memberChanged->isAnAdmin()) {
        if ( roomController->getCurrentUser() == memberChanged) {
            if (memberChanged == memberWhoChanged)
                logText(tr("You made yourself a room admin."), color);
            else
                logText(memberWhoChanged->getNickname()
                        + " " + tr("made you a room admin."), color);
        } else {
            if (memberChanged == memberWhoChanged)
                logText(memberChanged->getNickname() + " "
                        + tr("is now a room admin."), color);
            else
                logText(memberWhoChanged->getNickname() + " " + tr("made") + " "
                        + memberChanged->getNickname() + " " + tr("a room admin."),color);
        }

    } else
        logText(memberChanged->getNickname() + " " + tr("is no longer a room admin.")
                , color);

    int index = getUserBoxIndex(memberChanged);
    if (index == -1) {
        qDebug() << "[Chat] Someone changed admin status, but could not be found on the chat box.";
        return;
    }
    userBoxChangeUserIcon(index, getIcon(memberChanged));

    /* Update the ban button, because due to status change, the permission to ban might have changed: */
    UserButtonBox *widget = dynamic_cast<UserButtonBox*> (usersBox->widget(index));
    if ( (widget != NULL) && (widget->getUser() == memberChanged) )
        widget->updateBanButtonPermission();
}

void ChatRoom::nicknameUpdateFromUser(User *memberChanged, User *memberWhoChanged, QString oldName)
{
    if (memberChanged != memberWhoChanged)
        qDebug() << "[Chatroom] Nickname of " << oldName << " was changed by " << memberWhoChanged->getNickname();

    memberChanged->resetNicknameSufix();
    memberChanged->makeNameUnique( roomController->getPrimitiveListOfUsers() );
    updateName(memberChanged);
    if ( roomController->getCurrentUser() == memberChanged)
        addChatMessage(QString(tr("You changed nickname to") + " <b>" + memberChanged->getNickname()));
    else
        if (oldName != memberChanged->getNickname())
        addChatMessage(QString( oldName + " " + tr("changed nickname to") + " <b>" + memberChanged->getNickname() ));
}

void ChatRoom::dockWidgetOpened(QDockWidget *widget)
{
    if (widget == NULL)
        return;
    if (widget->widget()->inherits(PRIVATE_CHAT_BOX_NAME)) {
        PrivateChatBox *box = dynamic_cast<PrivateChatBox*> (widget->widget());
        box->focusToType();
        return;
    }

    if (widget->widget()->inherits(USER_BOX_NAME)) {
        if ( (latestUserBox != NULL) && (latestUserBox != widget) ) {
            emit tabifyDockWidget(latestUserBox, widget);
        }
        latestUserBox = widget;
        return;
    }

    if (widget->widget()->inherits(ACTION_LOG_NAME)) {
        if (actionLog != NULL)
            emit tabifyDockWidget(widget, dynamic_cast<QDockWidget*>(parent())); //tabify with chat
        return;
    }
}

/** Change the font of the chat (just for you): */
void ChatRoom::setChatFont(QFont font)
{
    chatText->setFont(font);
}

/** Get the icon that represents the user status: */
QIcon ChatRoom::getIcon(User *user)
{
    QIcon icon;
    if (user->isTheHost())
        icon = QIcon(":/images/hostIcon");
    else
    {
        if (user->isAnAdmin())
            icon = QIcon(":/images/adminIcon");
        else
            icon = QIcon(":/images/userIcon");
    }
    return icon;
}

/** Add a string to the chat box: */
void ChatRoom::appendChatMessage(QString message)
{
    chatText->append(message);
}

/** Change the username of a user in the list displaying who is in the chat: */
void ChatRoom::userBoxChangeUsernameTitle(int boxIndex, QString name)
{
    usersBox->setItemText(boxIndex, name);
}

/** Change the icon of a user in the list: */
void ChatRoom::userBoxChangeUserIcon(int boxIndex, QIcon newIcon)
{
    usersBox->setItemIcon(boxIndex, newIcon);
}

/** Send a message in the chat */
void ChatRoom::sendTextMessage()
{
    if (!chatLineEdit->text().isEmpty())
        roomController->sendMessageToServer( RoomController::MSG_CHAT_MESSAGE , (chatLineEdit->text()).toUtf8());
    chatLineEdit->setText(""); //Clean the text after message is sent.
    chatLineEdit->setFocus();
}

void ChatRoom::addUserToUserBox(User *user, UserButtonBox *userWidget, QString username)
{
    usersBox->addItem( userWidget , getIcon(user), username );

    /* Fix the userbox size based on the size of the widget inside it: */
    usersBox->setMinimumHeight( usersBox->minimumHeight() + TAB_HEIGHT_PER_USER);

    if (getTotalUsersInChat() == 0) { //First user
        usersBox->widget(0)->deleteLater();
        usersBox->removeItem(0);
    }
}

void ChatRoom::removeUserFromUserBox(User *user, int boxIndex)
{
    UserButtonBox *widget = dynamic_cast<UserButtonBox*> (usersBox->widget(boxIndex));
    usersBox->removeItem(boxIndex);

    /* Fix the userbox size based on the size of the widget inside it: */
    usersBox->setMinimumHeight( usersBox->minimumHeight() - TAB_HEIGHT_PER_USER);

    widget->deleteLater();
    if (DISPLAY_DEBUG_MESSAGES)
        qDebug() << user->getNickname() << "removed from the chat list.";
}

/** Get the index of the box where the list of users is stored: */
int ChatRoom::getUserBoxIndex(User *user)
{
    for (int i = 0; i < usersBox->count(); i++) {
        UserButtonBox *widget = dynamic_cast<UserButtonBox*> (usersBox->widget(i));
        if ( (widget != NULL) && (widget->getUser() == user) )
            return i;
    }
    return -1;
}

/** Connect the components that will display the elements of the view. */
void ChatRoom::linkViewComponents(QTextEdit *&chatText, QLineEdit *&chatLineEdit, QToolBox *&usersBox)
{
    chatText = ui->chatText;
    chatLineEdit = ui->chatLineEdit;
    usersBox = ui->usersBox;
}

/** Called when the "Send" button is pressed on the chat. */
void ChatRoom::on_sendMsgBtn_clicked()
{
    sendTextMessage();
}

/** When "Return" is pressed on the keyboard. */
void ChatRoom::on_chatLineEdit_returnPressed()
{
    sendTextMessage();
}

/** Retranslates the app if its language changed: */
void ChatRoom::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        refreshTitle();
        refreshActionLogTitle();
    }
    QWidget::changeEvent(event);
}
