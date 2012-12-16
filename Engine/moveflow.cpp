#include "moveflow.h"

MoveFlow::MoveFlow(QPoint initialPosition, QObject *parent) :
    QObject(parent)
{
    timerMilliseconds = 1000/FPS; //FPS should only be used here.
    timeElapsed = 0;
    delayBetweenMessages = 0;
    previousPosition = initialPosition;
    currentPosition = initialPosition;
    finalPosition = initialPosition;
    savedPosition = initialPosition;

    /* The timer signals every frame asking this class to send a move signal. */
    connect(&moveTimer, SIGNAL(timeout()), this, SLOT(sendMoveSignal()));

}

void MoveFlow::instantMove(QPoint position)
{
    previousPosition = position;
    currentPosition = position;
    finalPosition = position;
    savedPosition = position;
    emit move(currentPosition.toPoint());
}

void MoveFlow::moveTo(QPoint position)
{
    if (lastMoveReceived == position) return; //Avoid spam.
    lastMoveReceived = position;
    //qDebug() << "RECEIVED MOVE: " << position;

    if (position == finalPosition.toPoint()) return;

    if (  (timeElapsed <= timerMilliseconds*10) && (moveTimer.isActive()) ) {
        savedPosition = position;
        return;
    }

    delayBetweenMessages = timeElapsed;
    timeElapsed = 0; //reset time elapsed because there's a new position.

    previousPosition = finalPosition;
    finalPosition = position;

    if (!moveTimer.isActive()) {
        savedPosition = position;
        moveTimer.start(timerMilliseconds);
    } else
        savedPosition = QPoint();

}

void MoveFlow::sendMoveSignal()
{
    timeElapsed += timerMilliseconds; //Calculate how much time passed.

    if ( (timeElapsed >=  (timerMilliseconds*10 + delayBetweenMessages/2)) && (!savedPosition.isNull()) ) {
        lastMoveReceived = QPoint();
        moveTo(savedPosition);
    }

    currentPosition = calculatePosition(currentPosition, previousPosition, finalPosition);
    emit move(currentPosition.toPoint());

    if (timeElapsed > DELAY_MILLISECONDS) {
        moveTimer.stop();
        confirmPosition();
        return;
    }
}

void MoveFlow::confirmPosition()
{
    if (currentPosition.toPoint() != lastMoveReceived) {
        QPoint whereItShouldBe = lastMoveReceived;
        lastMoveReceived = QPoint();
        qDebug() << "[WARNING] DIFFERENT POSITIONS!";
        moveTo(whereItShouldBe);
    }
}


/**
 * @brief Calculate the next position of the object.
 * @return the position that the object should move to right now.
 */
QPointF MoveFlow::calculatePosition(QPointF start, QPointF previousEnd, QPointF end)
{
    QPointF position;
    qreal t = ( (qreal) timeElapsed) / (qreal) DELAY_MILLISECONDS;

    qreal coefficient = ( (qreal) delayBetweenMessages  ) / (qreal) DELAY_MILLISECONDS ;

    qreal xvelocity = (previousEnd.x() - start.x())*coefficient;
    qreal yvelocity = (previousEnd.y() - start.y())*coefficient;

    QPointF s1( start.x() + xvelocity, start.y() + yvelocity ); //Towards where you were going to.

    xvelocity = (end.x() - start.x())*coefficient;
    yvelocity = (end.y() - start.y())*coefficient;
    QPointF s2( end.x() - xvelocity, end.y() - yvelocity ); //Before where you are going now.

    /* Cubic splines: */

    position.setX(t*t*t * (end.x()  - 3*s2.x()     + 3*s1.x() - start.x())
                  + t*t * (3*s2.x() - 6*s1.x()     + 3*start.x())
                    + t * (3*s1.x() - 3*start.x()) + start.x() );
    position.setY(t*t*t * (end.y()  - 3*s2.y()     + 3*s1.y() - start.y())
                  + t*t * (3*s2.y() - 6*s1.y()     + 3*start.y())
                    + t * (3*s1.y() - 3*start.y()) + start.y() );

    return position.toPoint();
}
