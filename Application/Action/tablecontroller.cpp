#include "tablecontroller.h"

TableController::TableController(RoomView *room) :
    QObject(room)
{
    if (room == NULL) {
        qWarning() << "[TableController] Cannot initialize controller without a room.";
        return;
    }
    this->room = room;
    UDPMoveEnabled = true;

    /* Connect signals and slots from the table: */
    connectTableSignalAndSlots();
}

/** Make connections with the table: */
void TableController::connectTableSignalAndSlots()
{
    connect(room->table(), SIGNAL(requestImage(TableItem*)), this, SLOT(requestImage(TableItem*)));
    connect(room->table(), SIGNAL(itemMoved(TableItem*,QPoint,bool,bool)),
            this, SLOT  (itemMoved(TableItem*,QPoint,bool,bool)) );

    /* Requests from an item's context menu: */
    connect(room->table(), SIGNAL(requestRemoval(TableItem*)),
                                  this, SLOT  (removeItem(TableItem*)));
    connect(room->table(), SIGNAL(requestLockDialog(TableItem*)),
                                  this, SLOT  (openLockDialog(TableItem*)));
    connect(room->table(), SIGNAL(requestHideDialog(TableItem*)),
                                  this, SLOT  (openHideDialog(TableItem*)));
    connect(room->table(), SIGNAL(requestRotation(TableItem*,qint16)),
            this, SLOT  (rotateItem(TableItem*,qint16)));
    connect(room->table(), SIGNAL(requestOwnership(TableItem*,bool)),
            this, SLOT  (ownershipRequest(TableItem*,bool)));
    connect(room->table(), SIGNAL(requestShuffle(QList<TableItem*> )),
            this, SLOT  (shuffleRequest(QList<TableItem*> )));

}

/** Receive request to move item from the table: */
void TableController::itemMoved(TableItem *item, QPoint position, bool finalMove, bool sendMessage)
{
    /* Check if you have permission to move the item: */
    if (!Permission::hasObjectPermission(Permission::OBJECT_INTERACT, room->user(), room->info(), item)) {
        room->logText(tr("You don't have permission to move") + " \"" + item->getName() + "\"", Qt::gray);
        return;
    }

    /* Move the item (for you): */
    room->table()->moveItem(item, position);

    if (!finalMove) {
        if (!UDPMoveEnabled) sendMessage = false; //avoid spamming of udp moves
        else UDPMoveEnabled = false;
    }

    /* Warn others that the item was moved: */
    if (sendMessage)
        moveMessageSend(position, item->getItemId(), finalMove);
}

void TableController::removeItem(TableItem *item)
{
    /* Check if you have permission to remove the item: */
    if (!Permission::hasObjectPermission(Permission::OBJECT_REMOVE, room->user(), room->info(), item)) {
        room->logText(tr("You don't have permission to remove") + " \"" + item->getName() + "\"", Qt::gray);
        return;
    }

    /* Send network message asking the host for permission to remove item: */
    QByteArray package;
    QDataStream messageStream(&package, QIODevice::WriteOnly);
    messageStream << item->getItemId();
    emit sendActionMessageToHost(ActionController::ACTION_REQUEST_REMOVE_OBJECT, package);
}

void TableController::rotateItem(TableItem *item, qint16 rotation)
{
    /* Check if you have permission to rotate the item: */
    if (!Permission::hasObjectPermission(Permission::OBJECT_INTERACT, room->user(), room->info(), item)) {
        room->logText(tr("You don't have permission to rotate") + " \"" + item->getName() + "\"", Qt::gray);
        return;
    }

    QByteArray message;
    QDataStream messageStream(&message, QIODevice::WriteOnly);
    messageStream << item->getItemId();
    messageStream << rotation;
    emit sendActionMessage(ActionController::ACTION_ROTATE_OBJECT, message);
}

/** Send a message saying that the item was moved: */
void TableController::moveMessageSend(QPoint position, quint32 itemId, bool isTCP) {

    /* Declare a package to add all the variables you want to send: */
    QByteArray package;

    /* Declare a datastream to write the variables in the package: */
    QDataStream messageStream(&package, QIODevice::WriteOnly);

    /*
     * Add the data to the stream. The order that you
     * take variables out must match with the order you put variables
     * in. It is also important that all primitive types are
     * from "q" type, such as 'qint32' instead of just 'int':
     */
    messageStream << itemId;
    messageStream << position;

    /* You emit this signal to send a message to the server: */
    emit sendActionMessage(ActionController::ACTION_MOVE_OBJECT, package, TO_EVERYONE, isTCP);

    /*
       Ps: TO_EVERYONE is actually 0. If you want to send a message to a
       a specific user, you just have to replace TO_EVERYONE to the pointer
       of the User object who you want to send the message to.
     */
}

void TableController::handleRequestRemoveObject(QByteArray message, RoomMember *user)
{
    TableItem *item = Permission::userCanInteract(room, user, message, true, Permission::OBJECT_REMOVE);
    if (item == NULL) return;

        QByteArray package;
        QDataStream reply(&package, QIODevice::WriteOnly);
        reply << user->getUniqueID();
        reply << item->getItemId();
        emit sendActionMessage(ActionController::ACTION_REMOVE_OBJECT, package);
}

void TableController::handleRemoveObject(QByteArray message, RoomMember* user)
{
    if (user == NULL) return;
    if (!user->isTheHost()) return; //Can only receive this from host

    QDataStream messageStream(message);
    quint32 id;
    qint32 userID;
    messageStream >> userID;
    messageStream >> id; //item id
    TableItem *item =  (room->table()->getTableItem(id));
    if (item == NULL) return;
    room->table()->removeTableItem(item);

    RoomMember *from = room->user(userID);
    if (from) {
        QSound::play("Sound/itemRemove.wav");
        room->logText(from->getNickname() + " " + tr("removed item") + " \"" + item->getName() + "\"", Qt::darkRed);
    }
}

/* Request that comes from the Dialog Controller: */
void TableController::lockItem(TableItem *item, QList<qint8> teams)
{
    /* Check if you have permission to lock items: */
    if (!Permission::hasObjectPermission(Permission::OBJECT_LOCK_UNLOCK, room->user(), room->info(), item)) {
        room->logText(tr("You don't have permission to lock") + " \"" + item->getName() + "\"", Qt::gray);
        return;
    }

    /* Send network message to host requesting to lock the item: */
    emit sendActionMessageToHost(ActionController::ACTION_REQUEST_LOCK_OBJECT, getTeamPackage(item, teams));
}

/* Request that comes from the Dialog Controller: */
void TableController::hideItem(TableItem *item, QList<qint8> teams)
{
    /* Check if you have permission to hide items: */
    if (!Permission::hasObjectPermission(Permission::OBJECT_SHOW_HIDE, room->user(), room->info(), item)) {
        room->logText(tr("You don't have permission to hide") + " \"" + item->getName() + "\"", Qt::gray);
        return;
    }

    /* Send network message to host requesting to hide the item: */
    emit sendActionMessageToHost(ActionController::ACTION_REQUEST_HIDE_OBJECT, getTeamPackage(item, teams));
}

void TableController::openLockDialog(TableItem *item)
{
    qDebug() << "Open lock dialog requested";
    emit requestLockItemDialog(item);
}

void TableController::openHideDialog(TableItem *item)
{
    qDebug() << "Open hide dialog requested";
    emit requestHideItemDialog(item);
}

void TableController::handleRequestShowHideObject(QByteArray message, RoomMember *user)
{
    if (!Permission::userCanInteract(room, user, message, true, Permission::OBJECT_SHOW_HIDE)) return;
    QByteArray reply;
    QDataStream replyStream(&reply, QIODevice::WriteOnly);
    replyStream << user->getUniqueID();
    replyStream << message;
    emit sendActionMessage(ActionController::ACTION_HIDE_OBJECT, reply);
}

void TableController::handleRequestLockUnlockObject(QByteArray message, RoomMember *user)
{
    if (!Permission::userCanInteract(room, user, message, true, Permission::OBJECT_LOCK_UNLOCK)) return;
    QByteArray reply;
    QDataStream replyStream(&reply, QIODevice::WriteOnly);
    replyStream << user->getUniqueID();
    replyStream << message;
    emit sendActionMessage(ActionController::ACTION_LOCK_OBJECT, reply);
}

void TableController::handleRotateObject(QByteArray message, RoomMember *user)
{
    if (user == NULL) return;
    QDataStream messageStream(message);
    quint32 id;
    qint16 rotation;
    messageStream >> id;
    messageStream >> rotation;
    TableItem *item = room->table()->getTableItem(id);
    room->table()->rotateItem(item, rotation);
}

void TableController::handleShowHideObject(QByteArray message, RoomMember *user)
{
    setLockHideItem(message, user, false);
}

void TableController::handleLockUnlockObject(QByteArray message, RoomMember* user)
{
    setLockHideItem(message, user, true);
}

void TableController::handleShuffleRequest(QByteArray message, RoomMember *user)
{
    if (user == NULL) return;
    if (!room->user()->isTheHost()) return; //This function is exclusive to host

    QDataStream messageStream(message);
    qint32 totalItems;
    messageStream >> totalItems;
    QList<quint32> ids;
    QList<qreal> zPositions;
    qreal front = room->table()->getFrontZPosition();


    for (int i = 0; i < totalItems; i++) {
        quint32 itemID;
        messageStream >> itemID;
        TableItem *item = room->table()->getTableItem(itemID);
        if ( (item != NULL) && (Permission::hasObjectPermission(Permission::OBJECT_INTERACT,
                                                                user, room->info(), item)) ) {
            ids << itemID;
            qreal zPos = front + ((double)qrand() / (double)RAND_MAX);
            zPositions << zPos;
        }
    }

    totalItems = ids.size();

    QByteArray package;
    QDataStream replyStream(&package, QIODevice::WriteOnly);
    replyStream << user->getUniqueID();
    replyStream << totalItems;

    for (int i = 0; i < totalItems; i++) {
        replyStream << ids.at(i);
        replyStream << zPositions.at(i);
    }

    emit sendActionMessage(ActionController::ACTION_SHUFFLE, package);
}

void TableController::handleShuffle(QByteArray message, RoomMember *user)
{
    if (user == NULL) return;
    if (!user->isTheHost()) return; //Can only receive this from host

    qint32 totalItems;
    quint32 id, userID;
    qreal position;
    QDataStream messageStream(message);
    messageStream >> userID;
    messageStream >> totalItems;

    QSound::play("Sound/shuffle.wav");

    for (int i = 0; i < totalItems; i++) {
        messageStream >> id;
        messageStream >> position;
        TableItem *item = room->table()->getTableItem(id);
        if (item != NULL) room->table()->setZPosition(item, position);
    }

    RoomMember *owner = room->user(userID);
    if (owner == NULL) return;
    room->logText(owner->getNickname() + " " + tr("shuffled") + " "
                  + QString::number(totalItems) + " " + tr("items."), Qt::darkYellow);
}

void TableController::handleOwnershipRequest(QByteArray message, RoomMember *user)
{
    if (user == NULL) return;
    if (!room->user()->isTheHost()) return; //This function is exclusive to host

    QDataStream messageStream(message);
    quint32 id;
    bool requestToGetOwnership;
    messageStream >> id; //item id
    messageStream >> requestToGetOwnership;

    /* Make sure whoever sent the request is in the room: */
    RoomMember *owner = room->user(user->getUniqueID());
    if (owner == NULL) return;

    TableItem *item =  (room->table()->getTableItem(id));
    if (item == NULL) return;

    if ((Permission::canTakeOwnership(user, item, requestToGetOwnership)) ) {
        QByteArray package;
        QDataStream reply(&package, QIODevice::WriteOnly);
        reply << id;
        reply << requestToGetOwnership;
        if (requestToGetOwnership)
            reply << user->getUniqueID();
        emit sendActionMessage(ActionController::ACTION_OWNERSHIP, package);
    }
}

void TableController::handleOwnership(QByteArray message, RoomMember *user)
{
    if (user == NULL) return;
    if (!user->isTheHost()) return; //Can only receive this from host

    QDataStream messageStream(message);
    quint32 id;
    qint32 userID;
    bool requestToGetOwnership;
    messageStream >> id; //item id
    TableItem *item =  (room->table()->getTableItem(id));
    if (item == NULL) return;
    messageStream >> requestToGetOwnership;
    if (requestToGetOwnership) {
        messageStream >> userID;
        RoomMember *owner = room->user(userID);
        if (owner == NULL) return;
        item->setOwner(owner);
        if (owner->isThatYou())
            QSound::play("Sound/possessItem.wav");
        updatePermission(item);
        room->logText(owner->getNickname() + " " + tr("is now the owner of")
                      + " \"" + item->getName() + "\".", Qt::darkMagenta);
    } else {
        RoomMember *owner = room->user(item->getOwnerID());
        if (owner != NULL) {
            room->logText(owner->getNickname() + " " + tr("is not the owner of")
                      + " \"" + item->getName() + "\" " + tr("anymore."), Qt::magenta);
            if (owner->isThatYou())
                QSound::play("Sound/giveItemAway.wav");
        }
        item->cleanOwner();
        updatePermission(item);
    }
}

void TableController::shuffleRequest(QList<TableItem*> itemsToShuffle)
{
    QByteArray package;
    QDataStream message(&package, QIODevice::WriteOnly);
    qint32 totalItems = itemsToShuffle.size();
    message << totalItems;
    foreach (TableItem* item, itemsToShuffle) {
        message << item->getItemId();
    }

    /* Send the message to the host only: */
    emit sendActionMessageToHost(ActionController::ACTION_SHUFFLE_REQUEST, package);
}

void TableController::ownershipRequest(TableItem *item, bool takeOwnership)
{
    if (!Permission::canTakeOwnership(room->user(), item, takeOwnership)) return;
    QByteArray package;
    QDataStream message(&package, QIODevice::WriteOnly);
    message << item->getItemId();
    message << takeOwnership;

    /* Send the message to the host only: */
    emit sendActionMessageToHost(ActionController::ACTION_OWNERSHIP_REQUEST, package);
}

void TableController::setLockHideItem(QByteArray message, RoomMember *user, bool lockAction)
{
    if (user == NULL) return;
    if (!user->isTheHost()) return; //Can only receive this from host
    QDataStream packageStream(message);
    QByteArray package;
    qint32 userID;
    packageStream >> userID;
    packageStream >> package;
    QDataStream messageStream(package);
    quint32 id;
    QList<qint8> teams;
    messageStream >> id;
    messageStream >> teams;
    TableItem *item = room->table()->getTableItem(id);
    if (!item) return;
    bool wasLockedOrHidden;
    bool containsYourTeam = teams.contains( room->user()->getTeam() );
    QString logString;
    if (lockAction) {
        wasLockedOrHidden = item->isLocked(room->user()->getTeam());
        item->setLockedTeams(teams);
        if (wasLockedOrHidden != containsYourTeam) //If the status actually changed
            item->changeLockStatus(containsYourTeam);
        logString = tr("locked to"); //"John set object "MyImage" locked to Red, Blue."
    }
    else { //hideAction
        wasLockedOrHidden = item->isHidden(room->user()->getTeam());
        item->setHiddenTeams(teams);
        if (wasLockedOrHidden != containsYourTeam) //If the status actually changed
            item->changeHiddenStatus(containsYourTeam);
        logString = tr("as invisible to"); //"John set object "MyImage" as invisible to Red, Blue."
    }

    updatePermission(item);

    RoomMember *userWhoChanged = room->user(userID);
    if (userWhoChanged != NULL)
        room->logText(userWhoChanged->getNickname() + " " + tr("set item") + " \"" + item->getName() + "\" " +
                  logString + " " + getTeams(teams), Qt::darkCyan);

}

/** Called when permissions might have changed: */
void TableController::updatePermissions()
{
    foreach (TableItemDecorator *itemDecorator, room->table()->getTableItems())
        updatePermission(itemDecorator->getItem());
}

void TableController::updatePermission(TableItem *item)
{
    if (item == NULL) return;

    bool canSee = Permission::hasObjectPermission(Permission::OBJECT_SEE,
                                                   room->user(), room->info(), item);
    item->changeHiddenStatus(!canSee);

    bool canInteract = Permission::hasObjectPermission(Permission::OBJECT_INTERACT,
                                                   room->user(), room->info(), item);
    item->changeLockStatus(!canInteract);

    bool canDelete = Permission::hasObjectPermission(Permission::OBJECT_REMOVE,
                                                     room->user(), room->info(), item);
    item->removeSetEnabled(canDelete);

    bool canHide = Permission::hasObjectPermission(Permission::OBJECT_SHOW_HIDE,
                                                   room->user(), room->info(), item);
    item->hideUnhideSetEnabled(canHide);

    bool canLock = Permission::hasObjectPermission(Permission::OBJECT_LOCK_UNLOCK,
                                                   room->user(), room->info(), item);
    item->lockUnlockSetEnabled(canLock);
}

/** A method to handle messages about move actions (message arrived): */
void TableController::handleActionMove(QByteArray message, RoomMember *user)
{

    bool UDPMove = (user == NULL);
    /* UDP messages have NULL user. But your own move should still be refused by the table. */

    /* Put the byte array inside a QDataStream, so that the message can be opened: */
    QDataStream messageStream(message);

    /* Take out the variables in the same order that you put it in: */
    QPoint position;
    quint32 id;

    messageStream >> id;
    messageStream >> position; //get the new location from the message

    if (room->user()->isEqual(user)) { //You don't move since you already moved for yourself.
        UDPMoveEnabled = true;
        room->table()->bringToFront(room->table()->getTableItem(id));
        return;
    } else {
        if (UDPMove) UDPMoveEnabled = true; //Allow more udp moves since you received one.
    }

    if (! room->table()->moveItem(id, position, UDPMove) ) {
        if (user != NULL)
            qWarning() << "[TableController] Could not move item [" << id <<
                          "] Moved by " << user->getNickname();
    }
}

/** Send a message saying that there's a new object in the table: */
void TableController::sendActionAddObject(TableItem *item)
{
    /* Check again for permission (permission might have changed): */
    if (!Permission::canAddObjects(room->user(), room->info())) {
        emit addChatMessage(tr("You do not have permission to add an item anymore."), Qt::red);
        return;
    }

    QByteArray package;
    QDataStream messageStream(&package, QIODevice::WriteOnly);

     messageStream << item->getType();
     messageStream << item;

     item->deleteLater();

    /* Send the message to the host only: */
    emit sendActionMessageToHost(ActionController::ACTION_REQUEST_ADD_OBJECT, package);
}

/** Receive message that the AddObjectDialog of some user sent: */
void TableController::handleObjectReceived(QByteArray message, RoomMember *user)
{
    if (user == NULL) {
        qWarning() << "[TableController] Can't add object without a user.";
        return;
    }

    if (!user->isTheHost()) {
        qWarning() << "[TableController] Received a message that only the host could have sent.";
        return;
    }

    qint16 type; //The type of object received.
    QDataStream messageStream(message);
    messageStream >> type;

    TableItem *item = room->table()->newTableItem(type);
    messageStream >> item;

    qint32 userID;
    messageStream >> userID;

    /* Make sure whoever uploads the object is in the room: */
    RoomMember *owner = room->user(userID);
    if (owner == NULL) return;

    item->setOwner(owner);

    QSound::play("Sound/itemAdd.wav");

    /* Put the item in the table: */
    room->table()->addTableItem(item);

    /* Bring the item to the front: */
    room->table()->bringToFront(item);

    /* Recheck for object permissions: */
    updatePermission(item);

    /* Display in the log the action that happened: */
    room->logText(owner->getNickname() + " "
                  + tr("added item") + " \"" + item->getName() + "\"", Qt::darkGreen);
}

void TableController::handleObjectRequest(QByteArray message, RoomMember *user)
{
    if (!room->user()->isTheHost()) {
        qWarning() << "[TableController] Warning: Received an unexpected message to handle object request.";
        return;
    }

    if (user == NULL) {
        qWarning() << "[TableController] Can't add object without a user.";
        return;
    }

    if (!Permission::canAddObjects(user, room->info())) {
        qWarning() << user->getNickname() << " does not have permission to add images.";
        return;
    }

    /* Hide the welcome text from the room view (if it stills there ) */
    room->hideWelcomeText();

    qint16 type; //The type of object received.
    QDataStream messageStream(message);
    messageStream >> type;

    TableItem *item = room->table()->newTableItem(type);
    messageStream >> item;

    /* Set properties of the item: */
    room->table()->setItemID(item, room->user());
    if (item->pos() == QPoint(0,0))
        item->move(QPoint(250,150));

    /* Prepare reply message: */
    QByteArray reply;
    QDataStream replyStream(&reply, QIODevice::WriteOnly);
    replyStream << type;
    replyStream << item;
    replyStream << user->getUniqueID();

    /* Send the item to others: */
    emit sendActionMessage(ActionController::ACTION_ADD_OBJECT, reply);

    item->deleteLater();
}

/** Send image request. @param request if is to request image from others: */
void TableController::requestImage(TableItem *item)
{
    /* Check if you are not the only one in the room: */
    if (room->totalUsersInRoom() <= 1) {
        /* If yes, you can't request image from anyone */
        item->setErrorStatus(tr("Image not found at location:") + " <u>"
                             + item->getFilePath(IMAGE_FILE_TYPE)
                             + "</u>");
        return;
    }

    qDebug() << "[TableController] Image requested.";
    QByteArray package;
    QDataStream messageStream(&package, QIODevice::WriteOnly);
    messageStream << item->getItemId();
    messageStream << item->getFileHash();

    /* Send to everybody and wait for somebody's response: */
    emit sendActionMessage(ActionController::ACTION_IMAGE_REQUEST, package);
}

/** Receive image request: */
void TableController::handleActionRequestImage(QByteArray message, RoomMember *user)
{
    if (user == NULL) {
        qWarning() << "[TableController] This request can only be sent to identified users (privacy).";
        return;
    }


    if (room->user()->isEqual(user)) return; //You ignore your own message.

    quint32 itemId;
    QByteArray imageHash;

    QDataStream messageStream(message);
    messageStream >> itemId;
    messageStream >> imageHash;

    TableItem *item = room->table()->getTableItem(itemId);

    if (item == NULL) {
        qWarning() << "[TableController] Image request ignored because the item is not in the table.";
        return;
    }

    QPixmap image = item->getItemImage();
    bool imageFound = (!image.isNull());
    if (!imageFound) return; //Ignore because you also don't have the image.

    QByteArray packageInfo;
    QDataStream packageInfoStream(&packageInfo, QIODevice::WriteOnly);
    packageInfoStream << itemId;

    /* Send the message to the user saying if you have or not have the image requested: */
    emit sendActionMessage(ActionController::ACTION_IMAGE_REQUEST_REPLY, packageInfo, user);
}

void TableController::handleActionRequestImageReply(QByteArray message, RoomMember *user)
{
    if (user == NULL) {
        qWarning() << "[TableController] This request can only be sent to identified users (privacy).";
        return;
    }
    if (room->user()->isEqual(user)) return; //You ignore your own message.

    quint32 itemId;
    QDataStream messageStream(message);
    messageStream >> itemId;

    TableItemImage *item = dynamic_cast<TableItemImage*> (room->table()->getTableItem(itemId));

    if (item == NULL) {
        qWarning() << "[TableController] Image request ignored because the item is not in the table.";
        return;
    }

    if ( item->getStatus() == TableItem::STATUS_RECEIVING ) return; //Ignore message if you are already downloading an image.
    item->setReceivingStatus(0, user); //Alert that you are now downloading it.

    QByteArray packageInfo;
    QDataStream packageInfoStream(&packageInfo, QIODevice::WriteOnly);
    packageInfoStream << itemId;

    emit sendActionMessage(ActionController::ACTION_IMAGE_ORDER, packageInfo, user);
}

void TableController::handleActionOrderImage(QByteArray message, RoomMember *user)
{
    if (user == NULL) {
        qWarning() << "[TableController] Images can only be sent to identified users (privacy).";
        return;
    }

    quint32 itemId;

    QDataStream messageStream(message);
    messageStream >> itemId;

    TableItemImage *item = dynamic_cast<TableItemImage*> (room->table()->getTableItem(itemId));

    if (item == NULL) {
        qWarning() << "[TableController] Image request ignored because the item is not in the table.";
        return;
    }

    QByteArray packageInfo;
    QDataStream packageInfoStream(&packageInfo, QIODevice::WriteOnly);
    packageInfoStream << itemId;
    emit sendFile(ActionController::LARGE_MSG_IMAGE_REQUEST, item->getImagePath(), packageInfo, user);
}

/** Receive image (or progress of image) from the host user: */
void TableController::handleActionDownloadingImage(QByteArray message, RoomMember *user,
                                                   QByteArray largeMessage, double progress, bool starting)
{
    if (user == NULL) {
        qWarning() << "[TableController] For security reasons, you need a valid user to receive images.";
        return;
    }

    quint32 itemId;

    QDataStream messageStream(message);
    messageStream >> itemId;

    TableItemImage *item = dynamic_cast<TableItemImage*>(room->table()->getTableItem(itemId));
    if (item == NULL) {
        qDebug() << "[TableController] Could not find item in table. Image ignored.";
        return;
    }

    qDebug() << "[TableController] Handling image received.";

    if ( (!starting) && (item->getStatus() != TableItem::STATUS_RECEIVING) )
        return; //Ignore download piece if you weren't downloading the image.

    if (largeMessage.isEmpty()) {
        /* The image is still being downloaded, so display the progress: */
        item->setReceivingStatus((int) progress, user);

    } else

        /* Warn the table that the download is done: */
        room->table()->itemImageDownloaded(item, largeMessage);

}

/** Image being downloaded was cancelled for some reason: */
void TableController::handleActionDownloadingImageCancelled(QByteArray message, RoomMember *user)
{
    if (user == NULL) {
        qWarning() << "[TableController] For security reasons, you need a valid user to receive images.";
        return;
    }

    /* You were downloading an image and whoever was sending you the image stopped sending: */

    quint32 itemId;
    QDataStream messageStream(message);
    messageStream >> itemId;

    TableItemImage *item = dynamic_cast<TableItemImage*>(room->table()->getTableItem(itemId));
    if (item == NULL) {
        qDebug() << "[TableController] Could not find item in table. Message ignored.";
        return;
    }
    if ( item->getStatus() != TableItem::STATUS_RECEIVING ) return; //Ignore message if you weren't downloading the image.

    qDebug() << "[TableController] Download was cancelled. Requesting new download...";

    /* Ask again for an image since you are not downloading it anymore: */
    item->load();
}

QString TableController::getTeams(QList<qint8> teams)
{
    QList<QString> teamList = room->user()->getTeamList();
    QString teamsStr("");
    if (teams.isEmpty())
        teamsStr.append( tr("no one") ); //Set object invisible to no one.
    else {
        teamsStr.append( teamList.at(teams.takeFirst()) );
        foreach (qint8 team, teams)
            teamsStr.append( ", " + teamList.at(team));
    }
    teamsStr.append(".");
    return teamsStr;
}

QByteArray TableController::getTeamPackage(TableItem *item, QList<qint8> teams)
{
    QByteArray package;
    QDataStream messageStream(&package, QIODevice::WriteOnly);
    messageStream << item->getItemId();
    messageStream << teams;
    return package;
}
