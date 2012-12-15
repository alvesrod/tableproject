#ifndef TABLEITEMDECORATOR_H
#define TABLEITEMDECORATOR_H

#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QBitmap>
#include <QByteArray>
#include <QWidget>
#include <QObject>
#include <QPainter>
#include <QDebug>
#include "tableitem.h"
#include "Views/table.h"

class Table;

class TableItemDecorator : public QGraphicsProxyWidget
{

public:
    TableItemDecorator(TableItem *item, Table* table);
    bool stepItem(bool UDPMove, QPoint position);

    /** Called when the shape of an image item changed: */
    void refreshImageShape();

    /** Called when the table wants this clas to show that the item is locked: */
    void showHideLock(bool show);
    bool isShowingLock();

    void itemChangedRotation();

    /** Called (probably by the table) when the item changed his hidden status: */
    void hideStatusChanged(bool hidden);

    TableItem* getItem();

protected:

    QPainterPath shape() const;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    /** Draw the graphics when requested by Qt Libraries: */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget);


private slots:

    /** Called when it's time to clean up the lock feedback: */
    void lockFeedbackExpired();

private:
    void updateHideImage();
    QPixmap getRotatedItemImage(qint16 angle);
    QPainterPath createImageShape(qint16 angle);
    QPainterPath getShape(qint16 angle) const;


    TableItem* item;
    Table* table;

    /* True when it's to draw on the screen something telling the user that the item is locked: */
    bool displayLock;

    QByteArray fileHash;
    QPainterPath itemShape0;
    QPainterPath itemShape90;
    QPainterPath itemShape180;
    QPainterPath itemShape270;

    /* Variables used to calculate the move: */
    bool movingItem;
    bool currentUserWasLatestToMove;
    bool UDPMoveEnabled;
    QPoint offset;
    QPoint lastMoveSent;

    /* The image when the item is hidden: */
    QImage hiddenImage;
    QImage transformedHiddenImage;

    /* The image to give user feedback that the item is locked: */
    QImage lockedImage;

    QSize lastItemRect;
};


#endif // TABLEITEMDECORATOR_H
