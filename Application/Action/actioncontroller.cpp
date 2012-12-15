#include "actioncontroller.h"

ActionController::ActionController(RoomView *room):
    QObject(room)
{
    if (room == NULL) {
        qWarning() << "[ActionController] Cannot initialize controller without a room.";
        return;
    }

    this->room = room;
}

/*
 * ===========================================================================
 * ================ HANDLING MESSAGES RECEIVED FROM THE ROOM =================
 * ===========================================================================
 */

/** Called when you receive a message from the server: */
void ActionController::actionMessageReceived(QByteArray message, RoomMember *user, qint8 type)
{
    /* Deal with all the possible messages in the ActionMessageTypes enum: */
    switch (type) {

    /*
     * It's a good practice to deal with the action in a different method,
     * so, this method stays small. After a while, there could be hundreds of
     * actions here.
     */

    case ACTION_REQUEST_ADD_OBJECT:
        room->tableC()->handleObjectRequest(message, user);
        break;

    case ACTION_ADD_OBJECT:
        room->tableC()->handleObjectReceived(message, user);
        break;

    case ACTION_MOVE_OBJECT:
        room->tableC()->handleActionMove(message, user);
        break;

    case ACTION_IMAGE_REQUEST:
        room->tableC()->handleActionRequestImage(message, user);
        break;

    case ACTION_IMAGE_REQUEST_REPLY:
        room->tableC()->handleActionRequestImageReply(message, user);
        break;

    case ACTION_IMAGE_ORDER:
        room->tableC()->handleActionOrderImage(message, user);
        break;

    case ACTION_REQUEST_REMOVE_OBJECT:
        room->tableC()->handleRequestRemoveObject(message, user);
        break;

    case ACTION_REMOVE_OBJECT:
        room->tableC()->handleRemoveObject(message, user);
        break;

    case ACTION_ROTATE_OBJECT:
        room->tableC()->handleRotateObject(message, user);
        break;

    case ACTION_REQUEST_HIDE_OBJECT:
        room->tableC()->handleRequestShowHideObject(message, user);
        break;

    case ACTION_HIDE_OBJECT:
        room->tableC()->handleShowHideObject(message, user);
        break;

    case ACTION_REQUEST_LOCK_OBJECT:
        room->tableC()->handleRequestLockUnlockObject(message, user);
        break;

    case ACTION_LOCK_OBJECT:
        room->tableC()->handleLockUnlockObject(message, user);
        break;

    case ACTION_SHUFFLE_REQUEST:
        room->tableC()->handleShuffleRequest(message, user);
        break;

    case ACTION_SHUFFLE:
        room->tableC()->handleShuffle(message, user);
        break;

    case ACTION_OWNERSHIP_REQUEST:
        room->tableC()->handleOwnershipRequest(message, user);
        break;

    case ACTION_OWNERSHIP:
        room->tableC()->handleOwnership(message, user);
        break;

     /* More actions will come here. Add actions in the enum at the header. */

    default:
        qWarning() << "[ActionController] Warning: Received a message from an unknown type: " << type;
    }
}

/** Called when you receive a random value from the server: */
void ActionController::randomValuesReceived(qint32 min, qint32 max, qint16 type, QList<qint32> values, RoomMember *user)
{
    /* Deal with all the possible types of values in the RandomMessageTypes enum: */
    switch (type) {

    case RANDOM_THROW_DICE:
        handleDiceThrown(min, max, values, user);
        break;

     /* More random messages will come here. Add them in the enum at the header. */

    default:
        qWarning() << "[ActionController] Warning: Received a random value from an unknown type: " << type;
    }
}

/** Called when you receive a large message from the server: */
void ActionController::large_message_finished(qint8 type, QByteArray info, QByteArray package, RoomMember *user)
{
    qDebug() << "[RoomView] Large message download: finished. Size: " << package.size();
    switch (type) {

    case LARGE_MSG_IMAGE_REQUEST:
        room->tableC()->handleActionDownloadingImage(info, user, package, 0, false);
        break;

    default:
        qWarning() << "[RoomView] Received large message from unknown type: " << type;
    }
}

/** Called periodically to report progress when you are receiving a large message from the server: */
void ActionController::large_message_progress(qint8 type, QByteArray info, double percentage, RoomMember *user)
{
    qDebug() << "[RoomView] Large message download: " << percentage << "%";
    switch (type) {

    case LARGE_MSG_IMAGE_REQUEST:
        room->tableC()->handleActionDownloadingImage(info, user, 0, percentage, false);
        break;

    default:
        qWarning() << "[RoomView] Received large message from unknown type: " << type;
    }
}

void ActionController::large_message_cancelled(qint8 type, QByteArray info, RoomMember *user)
{
    qDebug() << "[RoomView] Large message download: Cancelled.";
    switch (type) {

    case LARGE_MSG_IMAGE_REQUEST:
        room->tableC()->handleActionDownloadingImageCancelled(info, user);
        break;

    default:
        qWarning() << "[RoomView] Received large message from unknown type: " << type;
    }
}

/** Called when you started receiving a large message from the server: */
void ActionController::large_message_started(qint8 type, QByteArray info, qint32 size, RoomMember *user)
{
    qDebug() << "[RoomView] Large message download: Started. Size: " << size;
    switch (type) {

    case LARGE_MSG_IMAGE_REQUEST:
        room->tableC()->handleActionDownloadingImage(info, user, 0, 0, true);
        break;

    default:
        qWarning() << "[RoomView] Received large message from unknown type: " << type;
    }
}

/*
 * ===========================================================================
 * ============== SLOTS CONNECTED TO SECONDARY ACTION CLASSES ================
 * ===========================================================================
 */

/** Public method called by a class that throws dices. */
void ActionController::throwDice(qint32 min, qint32 max, qint32 count)
{
    emit requestRandomValues(min, max, ActionController::RANDOM_THROW_DICE, count);
}

/*
 * ===========================================================================
 * ======================== PRIVATE HELPERS BELOW ============================
 * ===========================================================================
 */

/** A method to handle dice random values received from the server: */
void ActionController::handleDiceThrown(qint32 min, qint32 max, QList<qint32> values, RoomMember *user)
{
    /* Prepare a chat message displaying the results of the dice and who threw it: */
    QString diceMessage = user->getNickname() + " " + tr("threw a dice");
    QString diceValues = QString(" (") + QString::number(min) + ", " + QString::number(max) + "). ";
    QString diceResult = tr("Results") + ": " + QString::number(values.at(0));

    /* Remove the first value from the array because it was already used above: */
    values.removeFirst();

    /* Get all the random values and append them to the string: */
    foreach ( qint32 value, values)
        diceResult.append( ", " + QString::number(value) );

    /* Play dice sound: */
    QSound::play("Sound/dice.wav");

    /* Put all parts together and display the result of the dice in the chat: */
    emit addChatMessage(diceMessage + diceValues + diceResult + ".", Qt::blue);
}



