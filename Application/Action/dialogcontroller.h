#ifndef DIALOGCONTROLLER_H
#define DIALOGCONTROLLER_H

#include <QObject>
#include "EngineImpl/Views/roomview.h"
#include "Action/Views/throwdice.h"
#include "Action/Views/addobjectdialog.h"
#include "Action/Views/lockhidedialog.h"
#include "Action/Views/roominfodialog.h"

class RoomView;
class AddObjectDialog;
class LockHideDialog;
class RoomInfoDialog;

/**
 * @brief The DialogController class opens all dialogs that might
 * be opened as part of the action. It also connects signals to
 * those dialogs.
 */

class DialogController : public QObject
{
    Q_OBJECT
public:
    explicit DialogController(RoomView *room);

    /** Open the dialog that allows the user to add an object to the screen: */
    void openAddObjectDialog();

    /** Open the dialog that allows the user to throw a dice: */
    void openThrowDiceDialog();

    /** Open the dialog that displays information about the room: */
    void openRoomInfoDialog();

public slots:

    void openLockItemDialog(TableItem *item);

    void openHideItemDialog(TableItem *item);

private slots:
    void lockItemRequest(TableItem *item, QList<qint8> teams);
    void hideItemRequest(TableItem *item, QList<qint8> teams);

signals:
    void lockItem(TableItem *item, QList<qint8> teams);
    void hideItem(TableItem *item, QList<qint8> teams);
    
private:
    void getLockHideItemDialog();

    RoomView *room;

    /* Action Dialogs: */
    ThrowDice *diceDialog;
    RoomInfoDialog *roomInfoDialog;
    AddObjectDialog *addObjectDialog;
    LockHideDialog *lockHideDialog;
    
};

#endif // DIALOGCONTROLLER_H
