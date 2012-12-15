#ifndef CHATROOM_H
#define CHATROOM_H

#include <QDockWidget>
#include <QScrollBar>
#include <QKeyEvent>
#include <QTextEdit>
#include <QLineEdit>
#include <QToolBox>
#include <QWidget>
#include "Views/actionlog.h"
#include "Views/privatechatbox.h"
#include "Views/userbuttonbox.h"
#include "userbox.h"

/* Make this class "half" aware of the following Engine classes: */
class PrivateChatBox;
class UserButtonBox;
class ActionLog;

namespace Ui {
class ChatRoom; //The chat view
}

/**
 * @brief
 * This class implements the chatroom (with interface). The list of
 * users in the room is seen through this chat. This class can only
 * be accessed from inside the Engine. Any class outside the Engine
 * that wants to send a chat message should use methods in their own
 * superclass (assuming they are implementing an Engine class).
 */

#define PRIVATE_CHAT_BOX_NAME "PrivateChatBox" //Used to check if the dock widget opened inherits that
#define USER_BOX_NAME  "UserBox"
#define ACTION_LOG_NAME "ActionLog"
#define PRIVATE_CHAT_BOX_WIDTH 280
#define PRIVATE_CHAT_BOX_HEIGHT 160
#define SERVER_MESSAGE_COLOR "#990000" //The default color of server messages in the chatroom
#define DISPLAY_DEBUG_MESSAGES false
#define TAB_HEIGHT_PER_USER 24

class ChatRoom : public QWidget
{
    Q_OBJECT

public:
    ChatRoom(RoomController *controller, QWidget *parent = 0);
    ~ChatRoom();

    /*
     * Change the color of the chat messages that do not belong to a
     * specific user:
     */
    void setServerMessageColor(QString color);

    /* Get the icon that represents the user status: */
    static QIcon getIcon(User *user);

signals:

    /*
     * Open any dock widget. This can be used in multiple ways depending on the type of
     * application using this class:
     */
    void openDockWidget(QDockWidget *widget, Qt::DockWidgetArea area, bool isFloating = false);

    /* Tabify a dock widget: */
    void tabifyDockWidget(QDockWidget *first, QDockWidget *second);

    /* Sent when a row in the action log was selected: */
    void actionLogSelected(QModelIndex index);

public slots:

    /* This class deals with the private chat and user dock widget only.
       Other types of dock widget opened will just be ignored
       (they should be dealt in different classes, not in the chat).
    */
    void dockWidgetOpened(QDockWidget* widget);

    /* Add a chat message to the chat box: */
    void addChatMessage(QString message, User * user = NULL, QColor defaultColor = "");

    /* Add an item to the action log: */
    void addLog(QListWidgetItem *logItem);

    /* Add a text item to the action log: */
    void logText(QString text, QColor color);

protected:
    /** Retranslates the app if its language changed: */
    void changeEvent(QEvent* event);

    /** Below is just private helpers. It's not important. */
private slots:
    void privateChatMessageReceived(QByteArray message, User * user = NULL);
    void chatMessageReceived(QByteArray message, User * user = NULL);
    void privateMessageToBeSent(QString message, User *to);
    void someoneConnected(User *user);
    void someoneDisconnected(User * user);
    void chatFontChanged(QFont font);
    void adminStatusUpdateFromUser(User *memberChanged, User *memberWhoChanged);
    void privateMessageBtnClicked(User *user);
    void moreBtnClicked(User *user);
    void banBtnClicked(User *user);
    void logSelected(QModelIndex index);

    void nicknameUpdateFromUser(User *memberChanged, User *memberWhoChanged, QString oldName);
    void on_sendMsgBtn_clicked();
    void on_chatLineEdit_returnPressed();

private:
    int getUserBoxIndex(User *user);
    void appendChatMessage(QString message);
    void userBoxChangeUsernameTitle(int boxIndex, QString name);
    void userBoxChangeUserIcon(int boxIndex, QIcon newIcon);
    void setChatFont(QFont font);
    void sendTextMessage();
    void setUpChatroomUI();
    void refreshTitle();
    void refreshActionLogTitle();
    void setUserBoxUsername(User *user);
    int getTotalUsersInChat();
    void linkViewComponents(QTextEdit *&chatText, QLineEdit *&chatLineEdit, QToolBox *&usersBox);
    void declareConnections();
    QString getUserDisconnectedMessage(User *user);
    void updateName(User *user);
    QString getStringMessage(QString body, User *user = NULL, QString defaultColor = "");
    QString getMessageHead(User *user, QString defaultColor = "");
    PrivateChatBox* getPrivateMessageWidget(User *user, bool localRequest = true);
    UserBox* getUserBoxWidget(User *user);
    QDockWidget *setNewPrivateMessageWidget(User *user);
    QDockWidget *setNewUserBoxWidget(User *user);
    void addExistingUsersToChat();
    void addUserToChat(User *user);
    void setNewActionLogWidget();
    QString prepareUsernameForUserBox(User *user);
    QString getPrivateChatTitle(User *user);
    QString getUserBoxTitle(User *user);
    void removeUserFromUserBox(User *user, int boxIndex);
    void addUserToUserBox(User *user, UserButtonBox *userWidget, QString username);

    /* Private variables: */
    QHash<User*, QDockWidget*> privateChats;
    QHash<User*, QDockWidget*> userBoxes;
    QDockWidget* latestUserBox;
    ActionLog *actionLog;
    QDockWidget *logBox;
    RoomController *roomController;
    int totalUsers;
    QString serverMessageColor;
    QTextEdit *chatText;
    QLineEdit *chatLineEdit;
    QToolBox *usersBox;
    Ui::ChatRoom *ui;
};

#endif // CHATROOM_H
