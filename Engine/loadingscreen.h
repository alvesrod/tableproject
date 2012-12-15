#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include <QWidget>
#include <QTimer>
#include "Engine_global.h" /* This class can be used outside the Engine */
#include "roomcontroller.h"
#include "room.h"

/**
  * @brief
  * This is the loading screen that prepares everything before joining the room.
  * It loads the room and contacts the server to ask for permissions and to grab
  * all the information needed about the room (such as the current state of the
  * room and information about every user in there.
  */

#define DEFAULT_MAX_LOADING_TIME_SECONDS 20

class ENGINESHARED_EXPORT LoadingScreen : public QWidget
{
    Q_OBJECT

public:
    explicit LoadingScreen(Room *roomView, QWidget *parent = 0);

    enum LoadingSteps {
        LSTEP_ROOM_FOUND,
        LSTEP_ROOM_HOSTED,
        LSTEP_ROOM_CONNECTED,
        LSTEP_ROOM_PREPARED,
        LSTEP_ROOM_LOADED,
        LSTEP_ROOM_JOINED
    };

    /*
     * RoomController should be set after initializing this class.
     * This is not in the constructor because classes that will
     * inherit this one must not have access to this pointer.
     */
    void setRoomController(RoomController *controller);

    void startLoading(); //Called to start loading the screen.

signals:
    void openRoomView(); //call it to go to the room
    void goBackToMenu(); //call it to go back to the main menu
    void publishRoom(qint32 port); //add the room to the online list of rooms.

private slots:
    void connectedToServer(bool success);
    void serverStarted(bool success);
    void roomPrepared(bool success, QString result = "");
    void connectedToRoom(User* user);
    void loadingTimeout();

protected slots:
    /** Error messages that should be displayed in the screen: */
    virtual void errorWarning(QString warning) = 0;
    virtual void loadingStepComplete(LoadingSteps step) = 0;
    void setMaxLoadingTime(int seconds);

private:
    void connectToNetwork();
    void loadingStep(LoadingSteps step);
    void error(QString error);

    RoomController *controller;
    Room *room;
    int maxLoadingTime;
    bool loadingFailed;
    QTimer loadingTimer;
};

#endif // LOADINGSCREEN_H
