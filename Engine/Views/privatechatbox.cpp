#include "privatechatbox.h"
#include "ui_privatechatbox.h"

PrivateChatBox::PrivateChatBox(QWidget *parent, User *member) :
    QWidget(parent),
    ui(new Ui::PrivateChatBox)
{
    if (member == NULL) {
        qDebug() << "Private chat cannot be initialized without a member.";
        return;
    }
    user = member;
    ui->setupUi(this);
    setUsername();
}

PrivateChatBox::~PrivateChatBox()
{
    delete ui;
}

/**
 * Takes a string as a parameter and displays the string in the chat text box:
 */
void PrivateChatBox::addChatLine(QString line)
{
    ui->chatText->append(line); //Append to the bottom
}

/**
 *  Focus on the line edit so that users can type a msg after pressing "send"
 */
void PrivateChatBox::focusToType()
{
    ui->chatLineEdit->setFocus();
}

/**
 * Chat is disabled when the user you are talking to leaves the room.
 */
void PrivateChatBox::disableChat()
{
    ui->chatLineEdit->setText(""); //Erases the text you were typing.
    ui->chatLineEdit->setEnabled(false);
    ui->sendMsgBtn->setEnabled(false);
}

void PrivateChatBox::setChatFont(QFont font)
{
    ui->chatText->setFont(font);
}

/** Set the username displayed at the top of the window: */
void PrivateChatBox::setUsername()
{
    ui->titleLb->setText(tr("This is a private talk with ") + user->getNickname());
}

void PrivateChatBox::on_sendMsgBtn_clicked()
{
    sendTextMessage();
}

void PrivateChatBox::sendTextMessage()
{
    if (!ui->chatLineEdit->text().isEmpty())
        emit privateMessageToBeSent( ui->chatLineEdit->text(), user);

    ui->chatLineEdit->setText(""); //Clean the text after message is sent.
    focusToType();
}

void PrivateChatBox::on_chatLineEdit_returnPressed()
{
    sendTextMessage();
}

/** Retranslates the app if its language changed: */
void PrivateChatBox::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        setUsername();
    }
    QWidget::changeEvent(event);
}
