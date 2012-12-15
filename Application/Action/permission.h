#ifndef PERMISSION_H
#define PERMISSION_H

#include "EngineImpl/roommember.h"
#include "EngineImpl/roominfo.h"
#include "EngineImpl/Views/roomview.h"
#include "Action/tableitem.h"

class RoomInfo;
class RoomView;

/**
 * @brief The Permission class is used to determine
 * if a user has a permission to do something. Its
 * methods are static.
 */

class Permission
{
public:

    /** Types of permission that the user can have: */
    enum ObjectPermission {
        /*  0 */ OBJECT_SHOW_HIDE, //Permission to show or hide an object in the table
        /*  1 */ OBJECT_LOCK_UNLOCK, //Permission to lock or unlock and object in the table
        /*  2 */ OBJECT_SEE, //Permission to see an object in the table
        /*  3 */ OBJECT_REMOVE, //Permission to remove an object in the table
        /*  4 */ OBJECT_INTERACT //Permission to interact with an object, such as move, rotate, etc.
    };

    /** Return true if the user has permission to do something related to a table object. */
    static bool hasObjectPermission(ObjectPermission type, RoomMember *user, RoomInfo *roomInfo, TableItem *item);

    /** Return true if the user has permission to add objects to the table: */
    static bool canAddObjects(RoomMember *user, RoomInfo *roomInfo);

    /** Return true if the user has permission to become the owner of the item: */
    static bool canTakeOwnership(RoomMember *user, TableItem *item, bool wantOwnership);

    /** Return true if the user has permission to change teams: */
    static bool canChangeTeam(RoomMember *user, RoomInfo *roomInfo, bool yourOwnTeam);

    /** Checks the sender of the message to decide if he/she has permission: */
    static bool allowRequest(RoomInfo *roomInfo, RoomMember *you, RoomMember *sender,
                             bool hasToBeTheHost = false, bool checkObjectPermission = false,
                             TableItem *item = 0, ObjectPermission type = OBJECT_INTERACT);

    /** Return a valid table item if everything is ok with the message and the user has permissions: */
    static TableItem *userCanInteract(RoomView *room, RoomMember *user,
                                      QByteArray message, bool hasToBeTheHost = false,
                                      ObjectPermission type = OBJECT_INTERACT);


private:
    Permission();
};

#endif // PERMISSION_H
