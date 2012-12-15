#ifndef MOVEFLOW_H
#define MOVEFLOW_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include "Engine_global.h" /* This class can be used outside the Engine */

/**
 * @brief The MoveFlow class deals with lagged moves. It
 * receives move calls periodically and send more move
 * signals than it receives. The idea is that this class
 * make lagged moves flow from one point to the other (cubic
 * splines). It can be used by any object that moves in a 2D plane.
 * Ps: Once an object subscribes to MoveFlow, make sure the only
 * way this object moves around is through this class (this class
 * should be the only thing controlling the object moves).
 */

#define DELAY_MILLISECONDS 690 //The maximum delay in milliseconds that the move can be unsynchronized.
#define FPS 50 //Total of frames per second (or signals per seconds).

class ENGINESHARED_EXPORT MoveFlow : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Declare the MoveFlow. The class that uses it will subscribe to the move() signals.
     * @param initialPosition: The initial position of the object (x,y).
     */
    explicit MoveFlow(QPoint initialPosition, QObject *parent = 0);

    /**
     * @brief move instantly to a position (no flow).
     * @param position: the position that the object should move to.
     */
    void instantMove(QPoint position);

    /**
     * @brief request a move from one point to the other.
     * @param position: the position that the object should move to.
     */
    void moveTo(QPoint position);
    
signals:
    /** Signal emitted saying that the object should move to that position: */
    void move(QPoint position);

private slots:
    /** This slots is called in every frame asking for a move: */
    void sendMoveSignal();

private:
    /** Calculate the most suitable position for the object and return this position: */
    QPointF calculatePosition(QPointF currentLocation, QPointF previousEnd, QPointF finalLocation);

    /** Check if the position of the object is the expected position. Otherwise, move again: */
    void confirmPosition();

    QPointF previousPosition;
    QPointF currentPosition;
    QPointF finalPosition;
    QPoint savedPosition;
    QPoint lastMoveReceived;

    /* The difference in milliseconds between each signal sent. This is always (1000/FPS): */
    int timerMilliseconds;

    /* The time elapsed in milliseconds since the last move request: */
    int timeElapsed;

    int delayBetweenMessages;

    QTimer moveTimer;

};

#endif // MOVEFLOW_H
