#ifndef IENGINEFACTORY_H
#define IENGINEFACTORY_H

#include <QObject>
#include <QWidget>
#include <QMainWindow>

#include "Engine_global.h" /* This class can be used outside the Engine */

/* Make this class "half" aware of the following Engine classes: */
class Room;
class ViewController;
class RoomController;
class LoadingScreen;
class NewRoom;
class FindRoom;
class Menu;
class UserSettings;
class User;
class RoomDescription;
class UserBox;

/**
 * @brief
 * This class declares the classes that are going to inherit the Engine classes. The only class inside the Engine
 * aware of this class is the Engine class. This class should be implemented outside the Engine.
 *
 * Different apps using the same engine will declare different inheritances. This is an Abstract Factory.
 * It's an interface, so there's no declaration for this class.
 */

class ENGINESHARED_EXPORT IEngineFactory
{

public:

    virtual ViewController* newViewController(QWidget *parent = 0) = 0;

    virtual LoadingScreen* newLoadingScreen(Room *roomView = 0, QWidget *parent = 0) = 0;

    virtual Room* newRoomView(QWidget *parent = 0) = 0;

    virtual QWidget* newSettingsDialog(UserSettings *settings, QWidget *parent = 0) = 0;

    virtual Menu* newMainMenu(UserSettings *settings, QWidget *parent = 0) = 0;

    virtual User* newUser(qint32 id, QObject *parent , UserSettings *s, bool isHost = false) = 0;

    virtual UserBox* newUserBox(User *u, QWidget *parent = 0) = 0;

    virtual UserSettings* newUserSettings(QObject *parent = 0, bool isAnotherUser = false) = 0;

    virtual RoomDescription* newRoomDescription(qint32 port, QString ip = LOCAL_IP, QObject *parent = 0) = 0;

    virtual NewRoom* newNewRoom(UserSettings *settings, QWidget *parent = 0) = 0;

    virtual FindRoom* newFindRoom(UserSettings *settings, QWidget *parent = 0) = 0;

};

#endif // IENGINEFACTORY_H
