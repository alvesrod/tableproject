#include "permission.h"

Permission::Permission() {}

bool Permission::hasObjectPermission(Permission::ObjectPermission type, RoomMember *user, RoomInfo *roomInfo, TableItem *item)
{
    if ( (user == NULL) || (roomInfo == NULL) || (item == NULL) ) return false;
    qint8 team = user->getTeam();
    bool isOwnerOfObject = (item->getOwner()->isEqual(user));

    switch (type) {
    case OBJECT_SHOW_HIDE:
        if (item->isLocked(team)) return false; //Can't interact with an item that is locked.
        return user->hasPermission(roomInfo->getHideObjectPermission(), isOwnerOfObject);

    case OBJECT_LOCK_UNLOCK:
        return user->hasPermission(roomInfo->getLockObjectPermission(), isOwnerOfObject);

    case OBJECT_SEE:
        if (item->isHidden(team)) return false; //Can't see a hidden item.
        return true;

    case OBJECT_REMOVE:
        if (item->isLocked(team)) return false; //Can't interact with an item that is locked.
        return user->hasPermission(roomInfo->getRemoveObjectPermission(), isOwnerOfObject);

    case OBJECT_INTERACT:
        if (item->isLocked(team)) return false; //Can't interact with an item that is locked.
        return true;
    }

    return false;
}

bool Permission::canAddObjects(RoomMember *user, RoomInfo *roomInfo)
{
    if ( (user == NULL) || (roomInfo == NULL) ) return false;
    return user->hasPermission( roomInfo->getAddObjectPermission());
}

bool Permission::canTakeOwnership(RoomMember *user, TableItem* item, bool wantOwnership)
{
    if (user == NULL) return false;
    if (wantOwnership) { //User wants to become the owner
        if (item->hasOwner()) return false; //Item already has an owner.
    } else { //User doesn't want to be the owner anymore
        if (item->getOwnerID() != user->getUniqueID()) return false;
    }
    return true;
}

bool Permission::canChangeTeam(RoomMember *user, RoomInfo *roomInfo, bool yourOwnTeam)
{
    if ( (user == NULL) || (roomInfo == NULL) ) return false;
    return user->hasPermission( roomInfo->getChangeTeamsPermission(), yourOwnTeam);
}

/** Make sure the sender is valid: */
bool Permission::allowRequest(RoomInfo *roomInfo, RoomMember *you, RoomMember *sender, bool hasToBeTheHost,
                                     bool checkObjectPermission, TableItem *item, ObjectPermission type)
{
    /* Check for a valid sender: */
    if (sender == NULL) {
        qWarning() << "[Permission] Warning: Received a request from null user.";
        return false;
    }

    /* Check if message came from host user: */
    if ( (hasToBeTheHost) && (!you->isTheHost()) ) {
        qWarning() << "[Permission] Warning: Expecting message from host, but sender isn't host.";
        return false;
    }

    /* Check if user has permission to interact with object (if this is the case): */
    if (checkObjectPermission) {
        if (item == NULL) {
            qDebug() << "[Permission] Item not found in the table.";
            return false;
        }

        if (!Permission::hasObjectPermission(type, sender, roomInfo, item)) {
            qDebug() << "[Permission] User has no permission. (" << type << ").";
            return false;
        }
    }
    return true;
}

/** Open a message and check if the user has permission to interact with an item: */
TableItem *Permission::userCanInteract(RoomView *room, RoomMember *user,
                                       QByteArray message, bool hasToBeTheHost, ObjectPermission type)
{
    QDataStream messageStream(message);
    quint32 id;
    messageStream >> id; //item id
    TableItem *item =  (room->table()->getTableItem(id));
    if (!allowRequest(room->info(), room->user(), user, hasToBeTheHost, true, item, type))
        return NULL;
    return item;
}

