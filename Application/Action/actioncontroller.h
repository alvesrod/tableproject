#ifndef ACTIONCONTROLLER_H
#define ACTIONCONTROLLER_H

#include <QThread>
#include <QDebug>

#include "EngineImpl/Views/roomview.h"

class RoomView;

/**
 ** @brief Action Controller handles the messages that the Room receives.
 ** It is instantiated only in the RoomView class.
 **/

class ActionController : public QObject
{
    Q_OBJECT
public:
    ActionController(RoomView *room = 0);

    /**
     ** The types of messages that can be sent/received.
     ** This class will handle them:
     **/
    enum ActionMessageTypes {
        ACTION_REQUEST_ADD_OBJECT,
        ACTION_ADD_OBJECT,
        ACTION_MOVE_OBJECT,
        ACTION_IMAGE_REQUEST, //Ask everybody: "Who has this image? I don't have it"
        ACTION_IMAGE_REQUEST_REPLY, //Answer to who asked: "I have it!"
        ACTION_IMAGE_ORDER, //Answer back to the first who replied: "If you have it, send it to me".
        ACTION_REQUEST_REMOVE_OBJECT, //Ask the host to remove an object from the table.
        ACTION_REMOVE_OBJECT, //Response from host asking to remove object.
        ACTION_ROTATE_OBJECT,
        ACTION_REQUEST_HIDE_OBJECT,
        ACTION_HIDE_OBJECT,
        ACTION_REQUEST_LOCK_OBJECT,
        ACTION_LOCK_OBJECT,
        ACTION_SHUFFLE_REQUEST,
        ACTION_SHUFFLE,
        ACTION_OWNERSHIP_REQUEST,
        ACTION_OWNERSHIP

        /*
         * ADD HERE MORE MESSAGES!
         * YOU DEAL WITH THEM IN THE
         * actionMessageReceived() AND
         * sendMessage() METHODS.
         */
    };

    /**
     ** The types of random values that can be sent/received.
     ** This class will handle them:
     **/
    enum RandomMessageTypes {
        RANDOM_THROW_DICE

        /*
         * YOU DEAL WITH THEM IN THE
         * randomValueReceived() AND
         * requestRandomValue() METHODS.
         */
    };

    /**
     ** The types of large messages that can be sent/received.
     ** This class will handle them:
     **/
    enum LargeMessageTypes {
        LARGE_MSG_IMAGE_REQUEST

        /*
         * YOU DEAL WITH THEM IN THE
         * on_large_message_...() AND
         * sendLargeMessage() METHODS.
         */
    };
    
    /*
     * The methods below are called by their respective methods in the RoomView.
     * Their comments are in the RoomView class (check it out).
     */
    void actionMessageReceived(QByteArray message, RoomMember *user, qint8 type);
    void randomValuesReceived(qint32 min, qint32 max, qint16 type, QList<qint32> values, RoomMember *user);
    void large_message_started(qint8 type,  QByteArray info, qint32 size, RoomMember *user);
    void large_message_progress(qint8 type, QByteArray info, double percentage, RoomMember *user) ;
    void large_message_cancelled(qint8 type, QByteArray info, RoomMember *user);
    void large_message_finished(qint8 type, QByteArray info, QByteArray package, RoomMember *user);

signals:
    /* Those signals have their respective slots in the Room and Room View. Check them for comments: */
    void sendActionMessage(qint8 type, QByteArray message, User *user = 0, bool isTCP = TCP_MESSAGE);
    void sendActionMessageToHost(qint8 type, QByteArray message, bool isTCP = TCP_MESSAGE);
    void sendLargeMessage(qint8 type, QByteArray largePackage, QByteArray info, User *user = 0);
    void sendFile(qint8 type, QString filepath, QByteArray info, User *user = 0);
    void requestRandomValues(qint32 min, qint32 max, qint16 type, qint32 count = 1, User *to = 0, bool isTCP = TCP_MESSAGE);
    void addChatMessage(QString message, QColor color = "");

protected:
    void run();
    
private slots:

    /**
     * Method called by a class that throws dices.
     * @param min and max: boundaries of the dice.
     * @param count: total of dices to throw.
     */
    void throwDice(qint32 min, qint32 max, qint32 count = 1);

private:
    /** A method to handle dice random values received from the server: */
    void handleDiceThrown(qint32 min, qint32 max, QList<qint32> values, RoomMember *user);

    RoomView *room; //A pointer to the room.

};

#endif // ACTIONCONTROLLER_H
