#ifndef ENGINE_H
#define ENGINE_H

#include <QApplication>
#include <QTranslator>
#include <QDebug>
#include <QTime>
#include "Engine_global.h" /* This class can be used outside the Engine */
#include "enginefactoryinterface.h"
#include "viewcontroller.h"
#include "Views/updater.h"

/* Make this class "half" aware of the following Engine classes: */
class IEngineFactory;
class LoadingScreen;
class NewRoom;
class FindRoom;
class Room;
class UserSettings;
class Menu;
class User;
class RoomDescription;
class UserBox;

/**
  @brief This is the main class of the Engine.
  When Engine.start() is called, the GUI shows up and the app starts.
  This class should be called on the main().

  ==== THIS CLASS IS A SINGLETON ====
  It can only be initialized once per app.
 */


/*
 * Files with the name below are the ones that store app translations.
 * All translations must be set in the .pro file of the Application.
 * After updated, .qm files will be generated. Those .qm files should
 * be added to the TRANSLATION_FOLDER folder in the resource file.
 */
#define APP_TRANSLATION_NAME "apptr"
#define ENGINE_TRANSLATION_NAME "enginetr"
#define TRANSLATION_FOLDER "language/Translations"

/* Flag to warn that the application wants to be updated: */
#define UPDATE_FLAG "-update"


class ENGINESHARED_EXPORT Engine: public QObject
{
    Q_OBJECT
public:
    explicit Engine(IEngineFactory *factory, QString version = "",
                    QString secretKey = "", QString engineVersionCheck = "",
                    int argc = 0, char *argv[] = 0,
                    QString translationName = ""
                    );
    ~Engine();

    /** Starts the engine and display the GUI: */
    bool start();

    /**  Get a pointer to this object. @return NULL if the object wasn't declared: */
    static Engine* getInstance();

    /** @return true if Engine.start() was already called. */
    static bool isRunning();

    /** Get the view controller that does the swithing of views: */
    ViewController *getView();

    /** Change the language of the app: */
    static bool setAppLanguage(QString language);


    /* Static getters and setters for string data: */

    static QString getAppName();
    static void setAppName(QString name);

    static QString getAppCompany();
    static void setAppCompany(QString company);

    static QString getAppVersion();
    static void setAppVersion(QString version);

    static QString getAppSecret();
    static void setAppSecret(QString key);

    static QString getEngineVersion();


    /* Factory Methods that declare the public components of the Engine: */

    static LoadingScreen *newLoadingScreen(Room *roomView = 0, QWidget *parent = 0);

    static Room* newRoomView(QWidget *parent = 0);

    static QWidget* newSettingsDialog(UserSettings *settings, QWidget *parent = 0);

    static Menu* newMainMenu(UserSettings *settings, QWidget *parent = 0);

    static User* newUser(qint32 id, QObject *parent, UserSettings *s, bool isHost = false);

    static UserBox *newUserBox(User *u, QWidget *parent = 0);

    static UserSettings* newUserSettings(QObject *parent = 0, bool isAnotherUser = false);

    static RoomDescription* newRoomDescription(qint32 port, QString ip = LOCAL_IP, QObject *parent = 0);

    static NewRoom* newNewRoom(UserSettings *settings, QWidget *parent = 0);

    static FindRoom* newFindRoom(UserSettings *settings, QWidget *parent = 0);

private slots:
    /* Receives a signal asking to close the updater: */
    void closeUpdater();


private:
    static QString getLanguagePath();
    static void translateAppToDefaultLanguage();

    ViewController *view;
    Updater *updater;
    QString engineVersionCheck;

    bool updateApp;

    static QTranslator *appTranslator;
    static QTranslator *engineTranslator;
    static IEngineFactory *engineFactory;
    static Engine *singleton;
    static bool engineIsRunning;

    static QString appName;
    static QString appCompany;
    static QString appVersion;
    static QString appSecret;
    static QString appTranslationName;
    
};

#endif // ENGINE_H
