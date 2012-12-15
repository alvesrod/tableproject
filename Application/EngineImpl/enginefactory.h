#ifndef ENGINEFACTORY_H
#define ENGINEFACTORY_H

#include "../../Engine/enginefactoryinterface.h"
#include "../../Engine/viewcontroller.h"
#include "../../Engine/loadingscreen.h"

/**
 * @brief This class is a concrete factory. It implements IEngineFactory
 * to decide which classes are going to be used for the engine implementation.
 */

class EngineFactory : public IEngineFactory
{
public:

    /* The factory will implement those methods from IEngineFactory: */

    ViewController *newViewController(QWidget *parent = 0);

    LoadingScreen* newLoadingScreen(Room *roomView = 0, QWidget *parent = 0);

    Room* newRoomView(QWidget *parent = 0);

    QWidget* newSettingsDialog(UserSettings *settings, QWidget *parent = 0);

    Menu* newMainMenu(UserSettings *settings, QWidget *parent = 0);

    User* newUser(qint32 id, QObject *parent, UserSettings *s, bool isHost = false);

    UserBox* newUserBox(User *u, QWidget *parent = 0);

    UserSettings* newUserSettings(QObject *parent = 0, bool isAnotherUser = false);

    RoomDescription* newRoomDescription(qint32 port, QString ip = LOCAL_IP, QObject *parent = 0);

    NewRoom* newNewRoom(UserSettings *settings, QWidget *parent = 0);

    FindRoom* newFindRoom(UserSettings *settings, QWidget *parent = 0);
};

#endif // ENGINEFACTORY_H
