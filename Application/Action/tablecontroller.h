#ifndef TABLECONTROLLER_H
#define TABLECONTROLLER_H

#include <QObject>
#include "EngineImpl/Views/roomview.h"
#include "Action/tableitem.h"
#include "Action/Views/table.h"
#include "permission.h"

class TableItem;

/**
 * @brief The TableController class handles network
 * action messages destined to the table. This
 * is an intermediator between the Room and the Table.
 * The Room can still contact the Table if it's a local
 * change (that doesn't depend on network messages).
 */

class TableController : public QObject
{
    Q_OBJECT
public:
    explicit TableController(RoomView *room = 0);

    /** A method to handle messages about new items added to the table: */
    void handleObjectReceived(QByteArray message, RoomMember *user);

    /** A method asking the host permission to add an item to the table: */
    void handleObjectRequest(QByteArray message, RoomMember *user);

    /** A method to handle messages about move actions: */
    void handleActionMove(QByteArray message, RoomMember* user);

    /** Answer if you have the image: */
    void handleActionRequestImage(QByteArray message, RoomMember* user);

    /** Receive who has the image and decide who ask the image from: */
    void handleActionRequestImageReply(QByteArray message, RoomMember* user);

    /** Send the image to who ordered it. */
    void handleActionOrderImage(QByteArray message, RoomMember* user);

    /** A method to handle messages about the answer from the image request: */
    void handleActionDownloadingImage(QByteArray message, RoomMember* user,
                                      QByteArray largeMessage = 0, double progress = 0, bool starting = false);

    /** Called when a user was downloading an image, but the download was cancelled: */
    void handleActionDownloadingImageCancelled(QByteArray message, RoomMember* user);

    /** Send a message saying that the item was moved: */
    void moveMessageSend(QPoint position, quint32 itemId, bool isTCP);

    /** Received message asking permission to remove object: */
    void handleRequestRemoveObject(QByteArray message, RoomMember* user);

    /** Received message asking to remove object: */
    void handleRemoveObject(QByteArray message, RoomMember* user);

    /** Host received a message asking permission to show or hide an item: */
    void handleRequestShowHideObject(QByteArray message, RoomMember* user);

    /** Host received a message asking permission to lock or unlock an item: */
    void handleRequestLockUnlockObject(QByteArray message, RoomMember* user);

    /** Received message asking to rotate an item: */
    void handleRotateObject(QByteArray message, RoomMember* user);

    /** Received message asking to show or hide an item: */
    void handleShowHideObject(QByteArray message, RoomMember *user);

    /** Received message asking to lock or unlock an item: */
    void handleLockUnlockObject(QByteArray message, RoomMember* user);

    /** Host received a message asking permission to shuffle an item: */
    void handleShuffleRequest(QByteArray message, RoomMember* user);

    /** Received message asking to shuffle items: */
    void handleShuffle(QByteArray message, RoomMember* user);

    /** Host received a message asking permission to change ownership of an item: */
    void handleOwnershipRequest(QByteArray message, RoomMember* user);

    /** Received message asking to change ownership of an item: */
    void handleOwnership(QByteArray message, RoomMember* user);

    /** Set up what you can do with table items based on permissions you might have: */
    void updatePermissions();

public slots:

    /** Send a message saying that there's a new object in the table: */
    void sendActionAddObject(TableItem *item);

    /* Requests that come from the Dialog Controller: */
    void lockItem(TableItem *item, QList<qint8> teams);
    void hideItem(TableItem *item, QList<qint8> teams);

signals:
    /* Signals to send messages. The Room will capture them: */
    void sendActionMessage(qint8 type, QByteArray message, User *user = 0, bool isTCP = TCP_MESSAGE);
    void sendActionMessageToHost(qint8 type, QByteArray message, bool isTCP = TCP_MESSAGE);
    void sendLargeMessage(qint8 type, QByteArray largePackage, QByteArray info, User *user = 0);
    void sendFile(qint8 type, QString filepath, QByteArray info, User *user = 0);
    void addChatMessage(QString message, QColor color = "");
    void requestLockItemDialog(TableItem *item);
    void requestHideItemDialog(TableItem *item);
    
private slots:

    /** Received from a table signal asking for an image for the item: */
    void requestImage(TableItem *item);

    /** Received from a table signal saying that an item was moved: */
    void itemMoved(TableItem *item, QPoint position, bool finalMove = true, bool sendMessage = true);

    /** Update the permissions of the @param item: */
    void updatePermission(TableItem *item);

    /** Requests received from an item's context menu: */
    void removeItem(TableItem *item);
    void openLockDialog(TableItem *item);
    void openHideDialog(TableItem *item);
    void rotateItem(TableItem *item, qint16 rotation);
    void shuffleRequest(QList<TableItem *> itemsToShuffle);
    void ownershipRequest(TableItem *item, bool takeOwnership);

private:

    /** Subscribe to events from the table: */
    void connectTableSignalAndSlots();

    /** @return a string with all the names of the teams at @param team: */
    QString getTeams(QList<qint8> teams);

    /** Get a package to send to the network with the item id and teams: */
    QByteArray getTeamPackage(TableItem *item, QList<qint8> teams);

    /** Lock/unlock/hide/show an item based on the message: */
    void setLockHideItem(QByteArray message, RoomMember *user, bool lockAction);

    /** @return the item name or "?" if the item is hidden: */
    QString getItemNameIfAllowed(TableItem *item);

    RoomView *room;
    bool UDPMoveEnabled; //used to prevent spamming of move
};

#endif // TABLECONTROLLER_H
