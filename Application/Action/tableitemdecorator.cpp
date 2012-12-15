#include "tableitemdecorator.h"

TableItemDecorator::TableItemDecorator(TableItem *item, Table *table)
{
    if ( (item == NULL) || (table == NULL) ) {
        qWarning() << "[TableItemDecorator] Cannot initialize class without a table and an item.";
        return;
    }

    this->item = item;
    this->table = table;

    /* Add the TableItem to to the Decorator: */
    setWidget(item);

    displayLock = false;

    /* Variables to control move: */
    movingItem = false;
    UDPMoveEnabled = true;
    currentUserWasLatestToMove = false;

    /* Images for the lock and hide: */
    hiddenImage = QImage(":/images/hidden");
    lockedImage = QImage(":/images/protected");
    transformedHiddenImage = hiddenImage;

    /* Alert the superclass that the item might move: */
    setFlag(ItemIsMovable);

    setFlag(ItemSendsGeometryChanges);

}

bool TableItemDecorator::stepItem(bool UDPMove, QPoint position)
{
    if (UDPMove) {
        UDPMoveEnabled = true; //The previous UDP move was received, so you are allowed to send more.
        if (movingItem) return false; //Ignore messages from others if you are moving the object
        if (currentUserWasLatestToMove) return false; //Igore because it might be your past move that arrived late
    }
    currentUserWasLatestToMove = false;
    item->step(position);
    return true;
}

void TableItemDecorator::refreshImageShape()
{
    if (item->getItemImage().isNull()) return;

    if (fileHash != item->getFileHash()) {
        qDebug() << "Created mask";

        /* Shape isn't a good idea if the image is already too small: */
        QPixmap pix = item->getItemImage();
        if ((pix.height() < 17) && (pix.width() < 17))
            return;

        /* Create shapes for all 4 possible rotations: */
        itemShape0 = createImageShape(0);
        itemShape90 = createImageShape(90);
        itemShape180 = createImageShape(180);
        itemShape270 = createImageShape(270);

        fileHash = item->getFileHash();
        update();
    }
}


QPainterPath TableItemDecorator::createImageShape(qint16 angle)
{
    QPainterPath newShape;
    newShape.addRegion( QRegion( getRotatedItemImage(angle).mask()) );
    return newShape;
}


QPixmap TableItemDecorator::getRotatedItemImage(qint16 angle)
{
    QPixmap pix = item->getItemImage();
    if (angle == 0)
        return pix;

    QTransform trans;
    trans.rotate(angle);
    return pix.transformed(trans, Qt::SmoothTransformation);
}

void TableItemDecorator::showHideLock(bool show)
{
    displayLock = show;
    update();
}

bool TableItemDecorator::isShowingLock()
{
    return displayLock;
}

void TableItemDecorator::updateHideImage()
{
    QTransform trans;
    trans.rotate(item->getRotation());
    transformedHiddenImage = hiddenImage.transformed(trans, Qt::SmoothTransformation);
    lastItemRect = item->rect().size();
    if ( (hiddenImage.width() > item->rect().width()) || (hiddenImage.height() > item->rect().height()) )
        transformedHiddenImage = transformedHiddenImage.scaled(item->rect().size(),
                                                               Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void TableItemDecorator::itemChangedRotation()
{
    /* Change the rotation of the hidden image: */
    updateHideImage();
    update();
}

void TableItemDecorator::hideStatusChanged(bool hidden)
{
    /* Refresh the view: */
    if (hidden)
        updateHideImage();
    update();
}

TableItem *TableItemDecorator::getItem()
{
    return item;
}

QPainterPath TableItemDecorator::shape() const
{
    if ((!getShape(item->getRotation()).isEmpty()) && (!item->isHiddenForYou())) {
        return getShape(item->getRotation());
    }
    else
        return QGraphicsProxyWidget::shape();
}

void TableItemDecorator::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!movingItem) return;
    event->accept();

    QPoint nextMove = table->getCursorPosition() - offset;
    if (lastMoveSent == nextMove) return; //You already sent that one.
    lastMoveSent = nextMove;

    /*
     * UDPMoveEnabled is done to guarantee that one move message
     * will only be sent after the first one has arrived. So that,
     * we don't overload the network with move messages.
     */
    if (!item->checkLock())
        table->itemRequestMove(item, nextMove , false, UDPMoveEnabled);

    if (UDPMoveEnabled) UDPMoveEnabled = false;
}

void TableItemDecorator::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;
    if (item->checkLock()) return; //Can't move locked items
    event->accept();
    movingItem = true;
    currentUserWasLatestToMove = true;
    offset = table->getCursorPosition() - item->pos();
    table->itemRequestMove(item, item->pos(), true);
}

void TableItemDecorator::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;
    event->accept();
    offset = QPoint();
    if (!item->checkLock())
        table->itemRequestMove(item, item->pos(), true);
    movingItem = false;
}

void TableItemDecorator::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (item->checkLock()) return;
    event->accept();
    qint16 newAngle = item->getRotation() - 90;
    if (newAngle < 0) newAngle += 360;
    table->itemRequestRotation(item, newAngle);
}

void TableItemDecorator::paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget)
{

    /* Block the item content if it is hidden for you: */
    if ( (item->isHiddenForYou()) && (item->getStatus() == TableItem::STATUS_READY) ) {
        QBrush brush(Qt::SolidPattern);
        brush.setColor(QColor(0,0,0,5));
        painter->setBrush(brush);
        QPen pen(Qt::SolidLine);
        pen.setWidthF(1);
        pen.setColor(QColor(Qt::gray));
        painter->setPen(pen);
        painter->drawRect(item->rect());
        if (item->rect().size() != lastItemRect)
            updateHideImage();
        painter->drawImage(item->rect().width() / 2 - transformedHiddenImage.width() / 2,
                           item->rect().height() / 2 - transformedHiddenImage.height() / 2, transformedHiddenImage);
    } else
        QGraphicsProxyWidget::paint(painter, style, widget);

    /* Display a lock warning if the item asks for it: */
    if (displayLock) {
        QMatrix matrix;
        matrix.scale(1.0/table->getZoomScale(), 1.0/table->getZoomScale());
        matrix.rotate(-table->getTableRotation());
        painter->drawImage(item->rect().x() + item->rect().width() - 16 * (1.0/table->getZoomScale()), item->rect().y(),
                           lockedImage.transformed(matrix, Qt::SmoothTransformation) );
    }
}


QPainterPath TableItemDecorator::getShape(qint16 angle) const
{
    if (angle == 90)
        return itemShape90;
    if (angle == 180)
        return itemShape180;
    if (angle == 270)
        return itemShape270;

    return itemShape0;
}
