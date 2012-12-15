#include "enginefactory.h"
#include "roommember.h"
#include "membersettings.h"
#include "roominfo.h"
#include "mainwindow.h"
#include "Views/userdetails.h"
#include "Views/settingsdialog.h"
#include "Views/mainmenu.h"
#include "Views/loadingview.h"
#include "Views/roomview.h"
#include "Views/newroomdialog.h"
#include "Views/findroomdialog.h"


ViewController *EngineFactory::newViewController(QWidget *parent)
{
    return new MainWindow(parent);
}

LoadingScreen *EngineFactory::newLoadingScreen(Room *roomView, QWidget *parent)
{
    return new LoadingView(roomView, parent);
}

Room *EngineFactory::newRoomView(QWidget *parent)
{
    return new RoomView(parent);
}

QWidget *EngineFactory::newSettingsDialog(UserSettings *settings, QWidget *parent)
{
    return new SettingsDialog(settings, parent);
}

Menu *EngineFactory::newMainMenu(UserSettings *settings, QWidget *parent)
{
    return new MainMenu(settings, parent);
}

User* EngineFactory::newUser(qint32 id, QObject *parent, UserSettings *s, bool isHost)
{
    return new RoomMember(id, parent, s, isHost);
}

UserBox *EngineFactory::newUserBox(User *u, QWidget *parent)
{
    return new UserDetails(u, parent);
}

UserSettings *EngineFactory::newUserSettings(QObject *parent, bool isAnotherUser)
{
    return new MemberSettings(parent, isAnotherUser);
}

RoomDescription *EngineFactory::newRoomDescription(qint32 port, QString ip, QObject *parent)
{
    return new RoomInfo(port, ip, parent);
}

NewRoom *EngineFactory::newNewRoom(UserSettings *settings, QWidget *parent)
{
    return new NewRoomDialog(settings, parent);
}

FindRoom *EngineFactory::newFindRoom(UserSettings *settings, QWidget *parent)
{
    return new FindRoomDialog(settings, parent);
}
