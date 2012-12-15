#ifndef TABLEITEM_H
#define TABLEITEM_H

#include <QWidget>
#include <QPixmap>
#include <QFile>
#include <QDebug>
#include <QMouseEvent>
#include <QAction>
#include <QCryptographicHash>
#include <QMenu>
#include <QPainter>
#include <QHBoxLayout>

#include "../Engine/packagehandlerinterface.h"
#include "../Engine/moveflow.h"
#include "EngineImpl/roommember.h"
#include "constants.h"

class RoomMember;

#define DISPLAY_LOCK_MILLISECONDS 1000 //For how long the lock symbol will be displayed.

/**
 * @brief The TableItem class represents an item that can be added to the table.
 * It should be extended if you want a new type of item.
 */

class TableItem : public QWidget, public IPackageHandler
{
    Q_OBJECT
    
public:
    TableItem(QString name = "", QWidget *parent = 0);

    /* The types of status an item can be in: */
    enum ItemStatus {
        STATUS_PREPARING,
        STATUS_RECEIVING,
        STATUS_READY,
        STATUS_ERROR
    };

    /** Ask the item to move to @param position by moving step by step: */
    void step(QPoint position);

    /** Instantly move the item to the @param position: */
    void move(QPoint position);

    /**
     * The Table calls load() to load the item.
     * This method is called in the loading screen if
     * the item was already on the table before the user
     * arrives or when an item is added to a table.
     * This should be implemented by the class that inherits this one.
     * @param tableLoaded: true if the table was already loaded in the
     * loading screen. You can assume that if tableLoaded is false, it
     * is pointless to send network requests because they won't arrive.
     */
    virtual void load(bool tableLoaded = true) = 0;


    /* LOCK AND HIDE FEATURES: */

    void setHiddenTeams(QList<qint8> teams);
    QList<qint8> getHiddenTeams();
    bool isHidden(qint8 team);
    bool isHiddenForYou();

    void setLockedTeams(QList<qint8> teams);
    QList<qint8> getLockedTeams();
    bool isLocked(qint8 team);
    bool isLockedForYou();

    void changeHiddenStatus(bool hidden);
    void changeLockStatus(bool locked);

    /**
     * Returns true if the item is locked.
     * Differently from the method isLockedForYou(),
     * this one gives a feedback to the user if the
     * item is locked.
     */
    bool checkLock();

    /* ENABLING/DISABLING BUTTONS IN THE CONTEXT MENU: */

    void hideUnhideSetEnabled(bool enabled);
    void lockUnlockSetEnabled(bool enabled);
    void removeSetEnabled(bool enabled);
    void refreshSetEnabled(bool enabled);
    void rotationSetEnabled(bool enabled);
    void shuffleSetEnabled(bool enabled);

    /* GETTERS FOR THE CONTEXT MENU STATUS: */
    bool hideUnhideIsEnabled();
    bool lockUnlockIsEnabled();
    bool removeIsEnabled();

    /* MAKING BUTTONS IN THE CONTEXT MENU VISIBLE/INVISIBLE: */
    void refreshSetVisible(bool visible);

    /** Make an item have no owner: */
    void cleanOwner();

    /** Return true if the item has an owner: */
    bool hasOwner();

    /** Set an item owner: */
    void setOwner(RoomMember *user);

    /** Create the context menu (very self-explanatory): */
    void createContextMenu(QWidget *widget);

    /** Opens the context menu of an item at the mouse position: */
    void openContextMenu();

    /**
     * Returns the default path in the disk for a file of this item (if any).
     * @param fileType: The type of file (eg: .png or .txt).
     */
    QString getFilePath(QString fileType);


    /* GETTERS AND SETTERS: */

    QString getName();
    void setDirectory(QString tableDirectory);
    QString getDirectory();
    QByteArray getFileHash();
    void setFileHash(QByteArray hash);
    qint32 getOwnerID();
    RoomMember* getOwner();
    quint32 getItemId();
    void setItemId(quint32 id, RoomMember *user);
    qint16 getType();
    void setType(qint16 itemType);
    ItemStatus getStatus();
    QMenu* getContextMenu();
    qint16 getRotation();
    void setRotation(qint16 angle);
    qreal getZPosition();
    void setZPosition(qreal zPosition);
    QPixmap getItemImage();
    void setItemImage(QPixmap image);

    static QKeySequence getHideShorcut();
    static QKeySequence getLockShorcut();
    static QKeySequence getOwnerShorcut();
    static QKeySequence getRemoveShorcut();
    static QKeySequence getRemoveOwnerShorcut();

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

    /**
     * Save the contents of the @param fileData into the table item file.
     * @return an empty string if not saved or the path where the image was saved.
     */
    QString saveFile(QByteArray fileData, QString fileType);

    /** Called when the rotation happens: */
    virtual void onRotatingItem(qint16 rotation) = 0;

    void setPreparingStatus(); //Item is waiting to be shown.
    void setReceivingStatus(int progress, RoomMember *user); //Item is receiving a file to be shown.
    void setReadyStatus(); //Item is being displayed.
    void setErrorStatus(QString error); //Item cannot be displayed because of something.


protected:

    /* ITEM STATUS (TO BE IMPLEMENTED BY THE CLASS THAT INHERITS TABLEITEM): */

    virtual void onPreparingStatus() = 0;
    virtual void onReceivingStatus(int progress, RoomMember *user) = 0;
    virtual void onErrorStatus(QString error) = 0;
    virtual void onReadyStatus() = 0;

    /*
     * To be implemented by the class that inherits this one.
     * addDataToStream() is just inside getDataPackage() to add
     * more data to the package (such as new variables not implemented
     * in this class). useDataFromStream() is inside setDataPackage() and
     * behaves like it. The difference is that those 2 functions are implemented
     * by the class that inherits this one while getDataPackage() and setDataPackage()
     * are only implemented by this class.
     */
    virtual void addDataToStream(QDataStream &stream) = 0;
    virtual void useDataFromStream(QDataStream &stream) = 0;

    virtual void onItemHideChanged(bool hidden) = 0; //called when the item was hidden or shown to you;
    virtual void onItemLockChanged(bool locked) = 0; //called when the item was locked/unlocked for you;

    /** Get a sufix for the file and return it. One will be available. */
    int getAvailableFileExtension(QString fileExt);

    /** Retranslates the app if its language changed: */
    virtual void changeEvent(QEvent* event);

    /** Compose parameters to return the path to the disk file of this item (if any): */
    static QString makeFilepath(QString name, QString dir, QString fileExt, int extension = 0);

    void displayOwnerToolTip();

signals:
    /** Signal emmited when this class was updated based on a network package: */
    void dataUpdated();

    /** Signal emmitted when the oject ACTUALLY moves. */
    void moveFinalized();

    /** Warns the table that the user needs a feedback that the item is locked: */
    void requestLockFeedBack(TableItem *item, bool showLock);

    /** Warns the table that the user hid/showed this item: */
    void itemHideStatusChanged(TableItem *item, bool hidden);

    /**
     * Make a generic request to the table.
     * @param request is to be determined by the table and classes that inherits this one.
     */
    void tableRequest(TableItem *item, int request);

    /** Requests from the Context menu: */
    void requestRemoval(TableItem *item);
    void requestLockDialog(TableItem *item);
    void requestHideDialog(TableItem *item);
    void requestOwnership(TableItem *item, bool getOwnership);
    void requestShuffle(TableItem *item);
    void requestContextMenuFromTable(TableItem *item);
    void requestRotation(TableItem *item, qint16 rotation);

private slots:

    void lockFeedBackExpired();

    /* Called when the owner of the item change his/her nickname (to update): */
    void ownerChangedNickname(User* memberChanged, User*, QString);

    /* This should be the only class to contact move() */
    void moveReceived(QPoint position);

    /* Called when the user opens the context menu: */
    void contextMenuRequest(QPoint);

    /* Receiving signals from the Context Menu: */
    void onHideTriggered();
    void onLockTriggered();
    void onRemoveTriggered();
    void onRefreshTriggered();
    void onRotateTriggered();
    void onSetOwnerTriggered();
    void onRemoveOwnerTriggered();
    void onShuffleTriggered();

private:
    void handleOwnerContextMenu();

    /* The name of the item: */
    QString name;

    /* The Table Directory that this item belongs to: */
    QString directory;

    /* The unique ID of the item: */
    quint32 id;

    /* The type of the item (defined in classes that inherits this one): */
    qint16 type;

    /* The angle of rotation of the item: */
    qint16 rotation;

    /* The angle of rotation of the table: */
    qint16 tableRotation;

    /* The id of the owner of the item (or who added it): */
    qint32 ownerID;

    /* Control the moves of the item on the table: */
    MoveFlow *moveFlow;

    /* A hash value to identify the item file (if there's one): */
    QByteArray fileHash;

    /* List of teams that this item is hidden from: */
    QList<qint8> teamsHidden;

    /* List of teams that this item is locked from: */
    QList<qint8> teamsLocked;

    /* The context menu (when the user right-clicks an object: */
    QMenu *contextMenu;

    /* Buttons in the context menu: */
    QAction *hideUnhideAction;
    QAction *lockUnlockAction;
    QAction *setOwnerAction;
    QAction *removeOwnerAction;
    QAction *shuffleAction;
    QAction *refreshAction;
    QAction *removeAction;
    QAction *rotateAction;

    /* Private variables to check internally if the item is locked or hidden: */
    bool locked;
    bool hidden;

    /* The Z position of the item in the table: */
    qreal currentZPosition;

    /* The owner of the item: */
    RoomMember *owner;

    /* True if you are the owner of the image: */
    bool youAreTheOwner;

    QPixmap itemImage;

    bool showingLockFeedback;

    /* Status of the item in terms of downloading, error, etc. */
    ItemStatus status;


};

#endif // TABLEITEM_H
