#include "dialogcontroller.h"

DialogController::DialogController(RoomView *room) :
    QObject(room)
{
    this->room = room;

    /* Setting Action Dialogs to NULL: */
    diceDialog = NULL;
    addObjectDialog = NULL;
    lockHideDialog = NULL;
    roomInfoDialog = NULL;
}


/** Opens a dialog for the user to add an object in the screen: */
void DialogController::openAddObjectDialog()
{
    if ( !Permission::canAddObjects(room->user(), room->info()) ) {
        qWarning() << "[AddObjectDialog] You do not have permission to add images.";
        return;
    }

    if (!addObjectDialog) {
        addObjectDialog = new AddObjectDialog(room);

        /* Allow this dialog to add objects: */
        connect(addObjectDialog, SIGNAL(sendActionAddObject(TableItem*)),
                room->tableC(), SLOT(sendActionAddObject(TableItem*)), Qt::QueuedConnection);
    }
    addObjectDialog->showDialog();
}

/** Opens a dialog for the user to throw a dice: */
void DialogController::openThrowDiceDialog()
{
    if (!diceDialog) {
        diceDialog = new ThrowDice(room);

        /* Allow this dialog to roll a dice: */
        connect( diceDialog, SIGNAL(throwDice(qint32,qint32,qint32)),
                     room->action(), SLOT  (throwDice(qint32,qint32,qint32)) );
    }
    diceDialog->show();
    diceDialog->activateWindow();
}

void DialogController::openRoomInfoDialog()
{
    if (!roomInfoDialog)
        roomInfoDialog = new RoomInfoDialog(room);
    roomInfoDialog->showDialog();
}

void DialogController::openLockItemDialog(TableItem *item)
{
    getLockHideItemDialog();
    lockHideDialog->openLockDialog(item);
}

void DialogController::openHideItemDialog(TableItem *item)
{
    getLockHideItemDialog();
    lockHideDialog->openHideDialog(item);
}

void DialogController::lockItemRequest(TableItem* item, QList<qint8> teams)
{
    emit lockItem(item, teams);
}

void DialogController::hideItemRequest(TableItem* item, QList<qint8> teams)
{
    emit hideItem(item, teams);
}


void DialogController::getLockHideItemDialog()
{
    if (!lockHideDialog) {
        lockHideDialog = new LockHideDialog(room);

        connect( lockHideDialog, SIGNAL(hideItems(TableItem*, QList<qint8>)),
                     this, SLOT(hideItemRequest(TableItem*, QList<qint8>)));
        connect( lockHideDialog, SIGNAL(lockItems(TableItem*, QList<qint8>)),
                     this, SLOT(lockItemRequest(TableItem*, QList<qint8>)));
    }
}
