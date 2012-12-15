#ifndef TABLE_H
#define TABLE_H

#include <QWidget>
#include <QGraphicsItem>
#include <QHash>
#include <QGraphicsProxyWidget>
#include "Action/tableitem.h"
#include "Action/TableItems/tableitemimage.h"
#include "../../EngineImpl/Views/roomview.h"
#include "../Engine/packagehandlerinterface.h"
#include "Action/actioncontroller.h"
#include "Action/permission.h"
#include "Action/tableitemdecorator.h"

namespace Ui {
class Table;
}

class RoomInfo;
class TableItemImage;
class TableItemDecorator;

#define ZOOM_SCALE_LIMIT 500;
#define ZOOM_MIN 30
#define TABLE_RESIZE_COLDDOWN 20 //Wait that time in milliseconds before resizing the table

/**
 * @brief The Table class represents the table where all table objects can be added.
 * This is a view. The user can see this table and more its objects.
 */

class Table : public QWidget, public IPackageHandler
{
    Q_OBJECT
    
public:
    explicit Table(RoomInfo* roomInfo);
    ~Table();

    /* The types of items allowed in the table: */
    enum ItemTypes {
        ITEM_IMAGE
    };

    /* What the items in the table can request: */
    enum ItemRequest {
        ITEM_REQUEST_IMAGE, //Item wants an image from the network.
        ITEM_REQUEST_REFRESH_SHAPE //Item changed shape.
    };

    /** Called by the Room during loading screen asking to load the table: */
    void load();

    /**
     * Add an item to the table:
     * @param useItemPosition: true if it is to use the item's position.
     * false if it is to set a new position for the item.
     */
    void addTableItem(TableItem *item);

    /** Remove an item from the table: */
    bool removeTableItem(TableItem *item);

    /**
     * @brief moveItem tries to move an item from the table.
     * @param itemID: the id of the table item.
     * @param position: where to move the item to.
     * @param UDPMove: true if this move came from an UDP network message.
     * @return true if the item was moved.
     */
    bool moveItem(quint32 itemID, QPoint position, bool UDPMove);

    /** Instantly moves an item to @param position. It doesn't warn others: */
    void moveItem(TableItem *item, QPoint position);

    /** Brings an item to the front of all other items: */
    void bringToFront(TableItem *item);

    /** Sends an item to the back of all other items: */
    void sendToBack(TableItem *item);

    /** Get an item in the table with @param id. @return NULL if not found: */
    TableItem* getTableItem(quint32 id);

    /** Get the list of all items in the table: */
    QList<TableItemDecorator *> getTableItems();

    /** Instantiate an item based on its type: */
    TableItem* newTableItem(qint16 type);

    /** Warn the table that you downloaded an item image: */
    void itemImageDownloaded(TableItemImage *item, QByteArray package);

    /** Remove ownership of all items owner by @param user: */
    void removeUserOwnership(RoomMember *user);

    /** Receives a list of users and cleans ownership of a item if its owner is not in the list: */
    void fixItemOwnerships(QList<User*> usersInRoom);

    /* TABLE INTERACTIONS: */
    void zoomTable(int scale);
    void rotateTable(qint16 angle = 90);

    /** @return the angle of rotation of the table: */
    qint16 getTableRotation();

    /** Rotate a specific item on the table: */
    void rotateItem(TableItem *item, int angle = 90);

    /** Return the number of items currently in the table: */
    int totalOfItems();    

    /*
     * Methods from the PackageHandler interface that this class implements.
     * Those methods deal with all the shared class variables as a single ByteArray
     * package (necessary to send through the network). The operators << and >>
     * are overloaded to call those 2 functions. One prepares the package based on the
     * values of the variables. The other reads the package in the parameter and assign
     * values to the shared variables based on that package:
     */
    QByteArray getDataPackage();
    void setDataPackage(QByteArray message);

    /** Set a unique id for an item. An item needs an id to be added to the table: */
    void setItemID(TableItem* item, RoomMember *user);

    /** Returns the Z-Position of the object that is in the front of all: */
    qreal getFrontZPosition();

    /** Change an items' Z-position (to bring it to front / send it to back): */
    void setZPosition(TableItem *item, qreal z);

    /** Get the cursor position relative to the table: */
    QPoint getCursorPosition();

    qreal getZoomScale();

    /**
     * @brief itemMovedReceived is called by the item that wants to move.
     * @param item: the item that sent the signal.
     * @param position: where the item wants to move to.
     * @param finalMove: true if it is a final move (first or last move).
     * @param sendMessage: true if it is to send a network message warning others.
     */
    void itemRequestMove(TableItem* item, QPoint position, bool finalMove,  bool sendMessage = true);

public slots:
    void itemRequestRotation(TableItem *item, qint16 rotation);

signals:
    /** Signal emmited when this class was updated based on a network package: */
    void dataUpdated();

    /** Request an image for an item: */
    void requestImage(TableItem *item);

    /** Pass along to the Table Controller the move request received from the item: */
    void itemMoved(TableItem* item, QPoint position, bool finalMovem,  bool sendMessage = true);

    /** Requests from an item's context menu: */
    void requestRemoval(TableItem *item);
    void requestLockDialog(TableItem *item);
    void requestHideDialog(TableItem *item);
    void requestRotation(TableItem *item, qint16 rotation);
    void requestOwnership(TableItem *item, bool getOwnership);
    void requestShuffle(QList<TableItem*> itemsToShuffle);


protected:
    /** Called when the mouse wheel moved: */
    void wheelEvent(QWheelEvent *);

    /** Called when a keyboard key is pressed: */
    void keyPressEvent(QKeyEvent *key);

private slots:



    /** Receive a signal from the item when the move is done: */
    void itemMoved();

    /** Item request to give to the user feedback about it being locked: */
    void lockFeedBackRequested(TableItem *item, bool showLock);

    /**
     * Handles item requests.
     * @param item: the item who sent the request.
     * @param request: the request number (enum ItemRequest).
     */
    void itemRequest(TableItem *item, int request);

    /* Requests received from the context menu of a @param item: */
    void removeItemRequestReceived(TableItem *item);
    void hideDialogRequestReceived(TableItem *item);
    void lockDialogRequestReceived(TableItem *item);
    void ownershipRequestReceived(TableItem *item, bool getOwnership);
    void shuffleRequestReceived(TableItem *item);
    void contextMenuRequestReceived(TableItem *item);

    /** Called periodically by the colddown timer to allow table resize: */
    void timerExpiredAllowResizing();

    /** Called when the hide status of an item changed (eg: when the item was hidden). */
    void itemHideStatusChanged(TableItem *item, bool hidden);
    
private:

    /** Helper for the table zoom and rotation: */
    void updateScaleAndRotation();

    /** Subscribe to events from an item when added to the table: */
    void connectSignalAndSlots(TableItem *item);

    /** Adjust the size of the table based on its items: */
    void adjustTableSize(bool setTimer = true);

    /** @return the user if the owner of @param item is in the @param users: */
    User* ownerFound(QList<User*> users, TableItem *item);

    /** Get a list of items close to @param item to be able to shuffle them: */
    QList<TableItem*> getItemsToShuffle(TableItem *item);

    /** Get the item under the mouse position: */
    TableItem *getItemUnderMouse();

    /* True when the table is loaded on the loading screen: */
    bool tableLoaded;

    Ui::Table *ui;

    /* Stores the QGraphicsProxyWidget of all items in the table: */
    QHash<quint32, TableItemDecorator*> items;

    RoomInfo *roomInfo;

    qint16 zoomScale;
    qint16 rotation;

    /* Counter of the number of items added in total to be able to give item unique ids: */
    quint32 itemCounter;

    /* Current Z position of the item in the front (to calculate Z-Positions): */
    qreal zPositionFront;

    /* Z position of the item in the back: */
    qreal zPositionBack;

    /* The table scene inside the Graphics View: */
    QGraphicsScene * graphicsScene;

    bool allowTableResize;
    QTimer adjustTableColddownTimer;

};

#endif // TABLE_H
