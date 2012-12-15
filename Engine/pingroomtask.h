#ifndef PINGROOMTASK_H
#define PINGROOMTASK_H

/* Qt libraries: */
#include <QThread>
#include <QTime>
#include <QTimer>
#include <QEventLoop>

#include "Engine_global.h" /* This class can be used outside the Engine */
#include "roomcontroller.h"

/* Make this class "half" aware of the following Engine classes: */
class RoomController;

/*
 * When the Find Room dialog looks for a room, this is the time limit
 * to stop waiting for reply if the room does not respond:
 */
#define TIMEOUT_WAITING_TO_PING_ROOM_SECONDS 4


/**
 * @brief The main purpose of this thread is to try a new connection with a room just
 * to get more info about the room. Once the room has info ready, it emits a signal.
 * If there is no connection to the room, the thread will exit after a certain time.
 * The FindRoom class will use this task to fill up the list of opened rooms. This
 * class cannot be accessed from outside the Engine.
 */

class PingRoomTask : public QThread
{
    Q_OBJECT
public:
	/* 
	 * You need the ip and the port of the room to be able to start the task:
	 */
    explicit PingRoomTask(QString ip, int port, QObject *parent = 0);
    void startTask();

protected:
    void run();
signals:
	/*
	 * A result signal will ALWAYS be sent. It sends a null
	 * value if the result failed:
     */
    void result(RoomDescription *description, PingRoomTask *task);
    
private slots:
    void connectedToServer(bool success);
    void timeout();
    void roomHasInfoReady();

private:
    void sendResult(RoomDescription *description);

    QTime latencyTime;
    RoomController *controller;
    bool resultSent;
};

#endif // PINGROOMTASK_H
