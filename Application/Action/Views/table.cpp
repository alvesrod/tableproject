#include "table.h"
#include "ui_table.h"

Table::Table(RoomInfo *roomInfo) :
    QWidget(0),
    ui(new Ui::Table)
{
    this->roomInfo = roomInfo;

    ui->setupUi(this);
    itemCounter = 0;
    zoomScale = 250;
    rotation = 0;
    zPositionFront = 0;
    zPositionBack = 0;
    tableLoaded = false;
    allowTableResize = true;

    connect( &adjustTableColddownTimer, SIGNAL(timeout()),
             this, SLOT(timerExpiredAllowResizing()), Qt::QueuedConnection);

    /* Prepare the Scene of the table: */
    graphicsScene = new QGraphicsScene(this);
    ui->tableGraphicsView->setScene(graphicsScene);

    /* Necessary for the itemsBoundingRect() to return the x,y value expected: */
    ui->tableGraphicsView->scene()->addText("");

}

Table::~Table()
{
    /* Remove all table items: */
    if ( (ui->tableGraphicsView) && (ui->tableGraphicsView->scene()))
       ui->tableGraphicsView->scene()->clear();

    delete ui;
}

void Table::adjustTableSize(bool setTimer)
{
    /* To avoid adjusting the table all the time: */
    if (!allowTableResize) return;
    allowTableResize = false;

    /* The timer will set allowTableResize to true again. */
    if (setTimer)
        adjustTableColddownTimer.start(TABLE_RESIZE_COLDDOWN);

    qreal width, height, x, y;
    x = graphicsScene->itemsBoundingRect().x();
    y = graphicsScene->itemsBoundingRect().y();

    if ((graphicsScene->itemsBoundingRect().width() + x) < ui->tableGraphicsView->rect().width())
        width = ui->tableGraphicsView->rect().width() - 2*x;
    else
        width = graphicsScene->itemsBoundingRect().width();

    if ((graphicsScene->itemsBoundingRect().height() + y) < ui->tableGraphicsView->rect().height())
        height = ui->tableGraphicsView->rect().height() - 2*y;
    else
        height = graphicsScene->itemsBoundingRect().height();

    graphicsScene->setSceneRect(x, y, width,height);
    graphicsScene->update(x, y, width,height);
}

void Table::setZPosition(TableItem *item, qreal z)
{
    if (item == NULL) return;
    if (items.contains(item->getItemId())) {
        items[item->getItemId()]->setZValue(z);
        item->setZPosition(z);
    }
}

qreal Table::getZoomScale()
{
    int limit = ZOOM_SCALE_LIMIT;
    return (qPow(qreal(2), (zoomScale - limit/2) / qreal(50)));
}

/** Called after connections are set and data has arrived. Load the table: */
void Table::load()
{
    if (tableLoaded) {
        qWarning() << "[Table] Table can't load because it is already loaded.";
        return;
    }
    tableLoaded = true;

    /* Load each table item: */
    foreach (TableItemDecorator *item, items)
        item->getItem()->load(tableLoaded);

    adjustTableSize();
}

/** Handles item requests: */
void Table::itemRequest(TableItem *item, int request)
{
    switch (request) {
    case ITEM_REQUEST_IMAGE:
        emit requestImage(item);
        break;

    case ITEM_REQUEST_REFRESH_SHAPE:
        if (items.contains(item->getItemId()))
            items[item->getItemId()]->refreshImageShape();
        break;

    default:
        qWarning() << "[Table] Received an invalid request from an item. Request: " << request;
    }
}

/* Pass along to the table controller requests from an item's context menu: */
void Table::removeItemRequestReceived(TableItem *item) { emit requestRemoval(item); }
void Table::hideDialogRequestReceived(TableItem *item) { emit requestHideDialog(item); }
void Table::lockDialogRequestReceived(TableItem *item) { emit requestLockDialog(item); }
void Table::itemRequestRotation(TableItem *item, qint16 rotation) { emit requestRotation(item, rotation); }
void Table::ownershipRequestReceived(TableItem *item, bool getOwnership) { emit requestOwnership(item, getOwnership); }
void Table::shuffleRequestReceived(TableItem *item) { emit requestShuffle(getItemsToShuffle(item)); }

void Table::contextMenuRequestReceived(TableItem *item)
{
    if (!items.contains(item->getItemId())) return;
    item->shuffleSetEnabled( ((getItemsToShuffle(item).size() > 2) && (!item->isLockedForYou())) );
    item->openContextMenu();
}

void Table::lockFeedBackRequested(TableItem *item, bool showLock)
{
    if (!items.contains(item->getItemId())) return;
    if (items[item->getItemId()]->isShowingLock() == showLock) return;
    items[item->getItemId()]->showHideLock(showLock);
}

void Table::itemHideStatusChanged(TableItem *item, bool hidden)
{
    if (!items.contains(item->getItemId())) return;
    items[item->getItemId()]->hideStatusChanged(hidden);
}

QList<TableItem *> Table::getItemsToShuffle(TableItem *item)
{
    QList<TableItem*> returnList;
    if (!items.contains(item->getItemId())) return returnList;
    QList<QGraphicsItem*> widgets = graphicsScene->collidingItems(items[item->getItemId()]);

    foreach (QGraphicsItem* widget, widgets) {
        TableItemDecorator *itemWidget = dynamic_cast<TableItemDecorator*>(widget);
        if (itemWidget) {
            itemWidget->getItem();
            TableItem *item = itemWidget->getItem();
            if (!item->isLockedForYou())
                returnList << item;
        }
    }
    returnList << item;

    return returnList;
}

TableItem* Table::getItemUnderMouse() {
    bool gotOne = false;
    qreal front = 0;
    TableItem *item = NULL;
    foreach (TableItemDecorator *widget, items) {
        if (widget->isUnderMouse())
            if ( (widget->getItem()->getZPosition() > front) || (!gotOne) ) {
                item = widget->getItem();
                front = item->getZPosition();
                gotOne = true;
            }
    }
    return item;
}


void Table::timerExpiredAllowResizing()
{
    allowTableResize = true; //ajudstTableSize() will set this to false.
    adjustTableSize(false);
    allowTableResize = true; //set it to true again.
}

/** Instantiate a table item. This is a factory: */
TableItem *Table::newTableItem(qint16 type)
{
    TableItem *item = NULL;
    QString folder = roomInfo->getRoomFolderPath();

    switch (type) {
    case ITEM_IMAGE:
         item = new TableItemImage("Undefined", folder);
         break;

    default:
        qWarning() << "[Table] ERROR: Received an item from invalid type " << type;
    }

    item->setType(type);
    return item;
}

/** Add to items the image in the @package array: */
void Table::itemImageDownloaded(TableItemImage *item, QByteArray package)
{
    if (!item->loadImage()) {
        if (item->saveImage(package)) {
            if (!item->loadImage())
                item->setErrorStatus(tr("Could not load image received."));
            else {

                /* All items that were downloading the same image should also be updated: */
                foreach (TableItemDecorator *obj, items) {
                    if ( (obj->getItem()->getStatus() == TableItem::STATUS_RECEIVING)
                            && (obj->getItem()->getType() == ITEM_IMAGE)
                            && (obj->getItem()->getFileHash() == item->getFileHash())){

                        item = dynamic_cast<TableItemImage*>(obj->getItem());
                        if ( (item == NULL) || (!item->loadImage()) )
                            item->setErrorStatus(tr("Could not load image received."));
                    }
                }
            }
        }
        else
            item->setErrorStatus(tr("Could not save image received."));
    }
}

void Table::removeUserOwnership(RoomMember *user)
{
    if (user == NULL) return;
    foreach (TableItemDecorator* item, items) {
        if (item->getItem()->getOwnerID() == user->getUniqueID())
            item->getItem()->cleanOwner();
    }
}

void Table::fixItemOwnerships(QList<User *> usersInRoom)
{
    foreach (TableItemDecorator* item, items) {
        User* owner = ownerFound(usersInRoom, item->getItem());
        if (owner) item->getItem()->setOwner(dynamic_cast<RoomMember*>(owner));
        else item->getItem()->cleanOwner();
    }
}

User* Table::ownerFound(QList<User *> users, TableItem *item)
{
    foreach (User* user, users) {
        if (user->getUniqueID() == item->getOwnerID())
            return user;
    }
    return 0;
}

/** Add an item to the table: */
void Table::addTableItem(TableItem *item)
{
    if (item == NULL) {
        qWarning() << "[Table] Tried to add a NULL item to the table.";
        return;
    }
    if (item->getItemId() < 0) {
        qWarning() << "[Table] Tried to add an item with invalid id.";
        return;
    }

    /* Allow a context menu when the use right-clicks: */
    item->createContextMenu(this);

   TableItemDecorator* itemDecorator = new TableItemDecorator(item, this);

    /* Subscribe to events from the item: */
    connectSignalAndSlots(item);

    items[item->getItemId()] = itemDecorator;

    /* Let the item know that it is being added to the table: */
    item->load(tableLoaded);

    /* Set the size if the item is hidden: */
    if (item->isHiddenForYou())
        items[item->getItemId()]->hideStatusChanged(true);

    /* Add to table: */
    graphicsScene->addItem(itemDecorator);

    adjustTableSize();
}

/** Remove an item from the table: */
bool Table::removeTableItem(TableItem *item)
{
    if (item == NULL) return false;
    adjustTableSize();
    items.remove(item->getItemId());
    item->deleteLater();
    return true;
}

bool Table::moveItem(quint32 itemID, QPoint position, bool UDPMove)
{
    if (items.contains(itemID)) {
        if (items[itemID]->stepItem(UDPMove, position)) {
            if (!UDPMove) bringToFront(items[itemID]->getItem());
            return true;
        }
    }
    return false;
}

void Table::moveItem(TableItem *item, QPoint position)
{
    item->move(position);
}

void Table::bringToFront(TableItem *item)
{
    zPositionFront++;
    setZPosition(item, zPositionFront);
}

void Table::sendToBack(TableItem *item)
{
    zPositionBack--;
    setZPosition(item, zPositionBack);
}

TableItem *Table::getTableItem(quint32 id)
{
    if (!items.contains(id)) return NULL;
    return items[id]->getItem();
}

/** @return all items in the table: */
QList<TableItemDecorator *> Table::getTableItems()
{
    return items.values();
}

void Table::zoomTable(int scale)
{
    zoomScale += scale;

    int zoomLimit = ZOOM_SCALE_LIMIT;
    if (zoomScale > zoomLimit) zoomScale = zoomLimit;
    else if (zoomScale < ZOOM_MIN) zoomScale = ZOOM_MIN;

    updateScaleAndRotation();
}

void Table::rotateTable(qint16 angle)
{
    rotation = (rotation + angle) % 360;
    updateScaleAndRotation();
}

qint16 Table::getTableRotation()
{
    return rotation;
}

void Table::rotateItem(TableItem *item, int angle)
{
    if (!item) return;
    if (!items.contains(item->getItemId())) return;
    angle = angle % 360;
    item->setRotation(angle);
    item->onRotatingItem(angle);
    items[item->getItemId()]->itemChangedRotation();
}

int Table::totalOfItems()
{
    return items.size();
}

void Table::wheelEvent(QWheelEvent *)
{
/*
    Mouse wheel is disable for now.

    if (event->delta() < 0)
        zoomTable(-10);
    else
        zoomTable(10);
*/
}

void Table::keyPressEvent(QKeyEvent *key)
{

    if (QKeySequence(key->key()).matches(TableItem::getRemoveShorcut())) {
        TableItem* item = getItemUnderMouse();
        if ( (item) && (!item->checkLock()) && (item->removeIsEnabled()) )
            emit requestRemoval(item);
        return;
    }

    /* I'm only interested in ctrl events from now on: */
    if (!(key->modifiers() & Qt::ControlModifier))
        return;

    QKeySequence sequence(key->key() + Qt::CTRL);

    if (sequence.matches(TableItem::getLockShorcut())) {

        TableItem* item = getItemUnderMouse();
        if ( (item) && (item->lockUnlockIsEnabled()) )
            emit requestLockDialog(item);
        return;
    }

    if (sequence.matches(TableItem::getHideShorcut())) {
        TableItem* item = getItemUnderMouse();
        if ( (item) && (!item->checkLock()) && (item->hideUnhideIsEnabled()) )
            emit requestHideDialog(item);
        return;
    }

    if (sequence.matches(TableItem::getOwnerShorcut())) {
        TableItem* item = getItemUnderMouse();
        if (item) emit requestOwnership(item, true);
        return;
    }

    if (sequence.matches(TableItem::getRemoveOwnerShorcut())) {
        TableItem* item = getItemUnderMouse();
        if (item) emit requestOwnership(item, false);
        return;
    }

}

void Table::itemRequestMove(TableItem *item, QPoint position, bool finalMove, bool sendMessage)
{    
    emit itemMoved(item, position, finalMove, sendMessage);
}

/** One item just moved, so update the table size: */
void Table::itemMoved()
{
    adjustTableSize();
}

void Table::updateScaleAndRotation()
{
    qreal scale = getZoomScale();
    QMatrix matrix;
    matrix.scale(scale, scale);
    matrix.rotate(rotation);

    ui->tableGraphicsView->setMatrix(matrix);
}

void Table::connectSignalAndSlots(TableItem *item)
{
//    connect(item, SIGNAL(itemMoved(TableItem*,QPoint,bool,bool)),
//            this, SLOT  (itemMovedReceived(TableItem*,QPoint,bool,bool)) );

    connect(item, SIGNAL(tableRequest(TableItem*,int)),
            this, SLOT  (itemRequest(TableItem*,int)));

    connect(item, SIGNAL(requestRemoval(TableItem*)),
            this, SLOT  (removeItemRequestReceived(TableItem*)));

    connect(item, SIGNAL(requestHideDialog(TableItem*)),
            this, SLOT  (hideDialogRequestReceived(TableItem*)));

    connect(item, SIGNAL(requestLockDialog(TableItem*)),
            this, SLOT  (lockDialogRequestReceived(TableItem*)));

    connect(item, SIGNAL(requestRotation(TableItem*,qint16)),
            this, SLOT  (itemRequestRotation(TableItem*,qint16)));

    connect(item, SIGNAL(moveFinalized()),
            this, SLOT(itemMoved()), Qt::UniqueConnection);

    connect(item, SIGNAL(requestOwnership(TableItem*,bool)),
            this, SLOT(ownershipRequestReceived(TableItem*,bool)));

    connect(item, SIGNAL(requestShuffle(TableItem*)),
            this, SLOT(shuffleRequestReceived(TableItem*)));

    connect(item, SIGNAL(requestContextMenuFromTable(TableItem*)),
            this, SLOT(contextMenuRequestReceived(TableItem*)));

    connect(item, SIGNAL(requestLockFeedBack(TableItem*,bool)),
            this, SLOT(lockFeedBackRequested(TableItem*,bool)));

    connect(item, SIGNAL(itemHideStatusChanged(TableItem*,bool)),
            this, SLOT(itemHideStatusChanged(TableItem*,bool)));



    /*
     * Ps: UniqueConnection will avoid multiple calls. We just want to know if something moved
     * to be able to update the size of the table. We don't care how many items moved.
     */
}

QByteArray Table::getDataPackage()
{
    QByteArray message;
    QDataStream messageStream(&message, QIODevice::WriteOnly);
    qint16 totalItems = items.size();
    messageStream << itemCounter;
    messageStream << totalItems;
    messageStream << zoomScale;
    messageStream << rotation;
    messageStream << zPositionFront;
    messageStream << zPositionBack;

    foreach (TableItemDecorator* item, items) {
        messageStream << item->getItem()->getType();
        messageStream << item->getItem();
    }

    return message;
}

void Table::setDataPackage(QByteArray message)
{
    QDataStream messageStream(message);
    qint16 totalItems, type;
    messageStream >> itemCounter;
    messageStream >> totalItems;
    messageStream >> zoomScale;
    messageStream >> rotation;
    messageStream >> zPositionFront;
    messageStream >> zPositionBack;
    updateScaleAndRotation();

    qreal maxZPosition = zPositionFront;
    for (int i = 0; i < totalItems; i++) {
        itemCounter++;
        messageStream >> type;
        TableItem *item = newTableItem(type);
        messageStream >> item;
        addTableItem(item);
        qreal newPosition = zPositionFront + item->getZPosition();
        setZPosition(item, newPosition);
        if (newPosition > maxZPosition)
            maxZPosition = newPosition;
    }
    zPositionFront = maxZPosition;
    emit dataUpdated();
}

/** Set an id for an item. Only the host user can do it: */
void Table::setItemID(TableItem *item, RoomMember *user)
{
    itemCounter++;
    item->setItemId(itemCounter, user);
    qDebug() << "Item" << item->getName() << "has now ID:" << item->getItemId();
}

qreal Table::getFrontZPosition()
{
    return zPositionFront;
}

QPoint Table::getCursorPosition()
{
    QPoint cursor = QCursor::pos();

    if (getZoomScale() != 0 )
        cursor = cursor * (1.0 /getZoomScale());

    if (getTableRotation() == 0)
        return cursor;

    if (getTableRotation() == 90)
        return QPoint(cursor.y(), -cursor.x());

    if (getTableRotation() == 180)
        return QPoint(-cursor.x(), -cursor.y());

    return QPoint(-cursor.y(), cursor.x());
}


