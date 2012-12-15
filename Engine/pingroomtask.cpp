#include "pingroomtask.h"

PingRoomTask::PingRoomTask(QString ip, int port, QObject *parent)
{
    resultSent = false;
    
    /*
     * By passing "parent" we can guarantee that the RoomDescription will be deleted
     * once the parent of this class is deleted.
     */
    controller = new RoomController(Engine::newRoomDescription(port, ip, parent));

     /* RoomController steals the parenthood, but this time we dont want it.
      * If we don't set the parent, the room description will be deleted when we
      * delete the Room Controller in the end of this task:
      */
    controller->getRoomDescription()->setParent(parent);

    connect(controller, SIGNAL(connectedToServer(bool)), this, SLOT(connectedToServer(bool)), Qt::QueuedConnection);
    connect(controller, SIGNAL(roomHasInfoReady()), this, SLOT(roomHasInfoReady()), Qt::QueuedConnection);

    /*
     * Set a timeout to delete this task if it got no response:
     */
    QTimer::singleShot(TIMEOUT_WAITING_TO_PING_ROOM_SECONDS*1000, this, SLOT(timeout()));

    /* Delete this thread when it is finished: */
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void PingRoomTask::startTask()
{
    qDebug() << "Task started";
    start();
    controller->connectToServer();
}

void PingRoomTask::timeout()
{
    sendResult(NULL);
}

void PingRoomTask::run()
{
    exec();
}

void PingRoomTask::connectedToServer(bool success)
{
    if (resultSent) return;

    if (success) {
        latencyTime = QTime::currentTime();
        
        /*
         * Once you call REQUEST_ROOM_INFO, the RoomController emits a signal
         * "roomHasInfoReady()" when you loaded the room info. This signal might
         * never arrive because it needs to go to the server to get the room info,
         * and the server can just not respond.
         */
        controller->sendMessageToServer(RoomController::MSG_REQUEST_ROOM_INFO);
    } else {
        sendResult(NULL);
        qDebug() << "Room pinged could not connect to server.";
    }
}

/**
 * Ok, the info about the room arrived.
 * Time to emit a signal with the results:
 */
void PingRoomTask::roomHasInfoReady()
{    
    if (resultSent) return;

    RoomDescription *description = controller->getRoomDescription();
    /*
     * Latency is the difference in miliseconds from when the message was
     * requested to when the message arrived:
     */
    description->setRoomLatency( latencyTime.msecsTo(QTime::currentTime()) );

    sendResult(description);
}

void PingRoomTask::sendResult(RoomDescription *description)
{
    if (!resultSent) {
        resultSent = true;
        controller->deleteLater();
        qDebug() << "Room returned result.";
        emit result(description, this);
        this->exit();
    }
}

