#ifndef PRIVATECHATBOX_H
#define PRIVATECHATBOX_H

#include <QWidget>
#include <QDebug>
#include "../user.h"

namespace Ui {
class PrivateChatBox;
}

/**
 * @brief This is the Private Chat window that pops up when the user wants to send
 * a private message to another user. This is called by the ChatRoom and cannot
 * be accessed outside the Engine.
 */

class PrivateChatBox : public QWidget
{
    Q_OBJECT
    
public:
    PrivateChatBox(QWidget *parent, User *member);
    ~PrivateChatBox();
    void addChatLine(QString line);
    void focusToType();
    void disableChat();
    void setChatFont(QFont font);
    void setUsername();

signals:
    void privateMessageToBeSent(QString message, User *to);

protected:
    /** Retranslates the app if its language changed: */
    void changeEvent(QEvent* event);
    
private slots:
    void on_sendMsgBtn_clicked();

    void on_chatLineEdit_returnPressed();

private:
    PrivateChatBox(QWidget *parent = 0);
    void sendTextMessage();
    Ui::PrivateChatBox *ui;
    User *user;
};

#endif // PRIVATECHATBOX_H
