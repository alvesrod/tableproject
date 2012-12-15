#include "engine.h"

/*
 * THIS CLASS IS A SINGLETON.
 * All variables below are static:
 */
bool Engine::engineIsRunning = false;
QString Engine::appTranslationName = APP_TRANSLATION_NAME;

Engine* Engine::singleton             = NULL;
IEngineFactory* Engine::engineFactory = NULL;
QTranslator* Engine::appTranslator    = NULL;
QTranslator* Engine::engineTranslator = NULL;

QString Engine::appName    =  "[NO-APP-NAME-SET]";
QString Engine::appCompany =  "[NO-APP-COMPANY-SET]";
QString Engine::appVersion =  "[NO-APP-VERSION-SET]";
QString Engine::appSecret  =  "[NO-APP-SECRET-SET]";


Engine::Engine(IEngineFactory *factory, QString version, QString secretKey,
               QString engineVersionCheck, int argc, char *argv[], QString translationName):
    QObject()
{
    if (singleton != NULL) {
        qWarning() << "WARNING: The Engine can only be initialized once!";
        this->deleteLater();
        return;
    }

    if (factory == NULL) {
        qWarning() << "WARNING: The Engine requires a factory.";
        this->deleteLater();
        return;
    }

    updater = NULL;

    /* If there is an update argument being passed, it means "update!" */
    updateApp = false;
    for (int i = 0; i < argc; i++) {
        if (QString(argv[i]) == QString(UPDATE_FLAG)) {
            qDebug() << "The application will be updated.";
            updateApp = true;
        }
    }

    this->engineVersionCheck = engineVersionCheck;

    singleton = this;
    engineFactory = factory;
    view = NULL;

    if (!version.isEmpty())
        appVersion = version;

    if (!translationName.isEmpty())
        appTranslationName = translationName;

    if (!secretKey.isEmpty())
        appSecret = secretKey;

    appCompany = QApplication::organizationName();
    appName = QApplication::applicationName();

    /* Translate the app based on language chosen before or the operational system's language: */
    translateAppToDefaultLanguage();

    /* Sets the seed, so that every class can use qrand(). */
    qsrand(QTime::currentTime().msec());
}

Engine::~Engine()
{
    engineIsRunning = false;

    if (appTranslator != NULL)
        appTranslator->deleteLater();

    if (engineTranslator != NULL)
        engineTranslator->deleteLater();

    if (singleton != NULL)
        singleton->deleteLater();

    if (view != NULL)
        view->deleteLater();

    if (updater != NULL)
        updater->deleteLater();
}

QString Engine::getAppName()
{
    return appName;
}

void Engine::setAppName(QString name)
{
    appName = name;
}

QString Engine::getAppCompany()
{
    return appCompany;
}

void Engine::setAppCompany(QString company)
{
    appCompany = company;
}

QString Engine::getAppVersion()
{
    return appVersion;
}

void Engine::setAppVersion(QString version)
{
    appVersion = version;
}

QString Engine::getAppSecret()
{
    return appSecret;
}

void Engine::setAppSecret(QString key)
{
    appSecret = key;
}

ViewController *Engine::getView()
{
    return view;
}

Engine *Engine::getInstance()
{
    return singleton;
}

bool Engine::isRunning()
{
    return engineIsRunning; //true if start() was called.
}

/**
 * Called to start the engine:
 * @return true if the engine started.
 */
bool Engine::start()
{
    if (engineIsRunning) {
        qDebug() << "[Engine] Engine was already started().";
        return false;
    }

    engineIsRunning = true;

    /* If there is an update argument being passed, it means "update!" */
    if (updateApp) {
            updater = new Updater(0); /* Update the application */
            connect(updater, SIGNAL(signalClose()), this, SLOT(closeUpdater()), Qt::QueuedConnection);
            updater->startUpdate();
            return true;
    }

        /* Display the GUI: */
        view = engineFactory->newViewController();

        if (engineVersionCheck != QString(ENGINE_VERSION)) {
            QMessageBox::critical(view, tr("Error"), tr("The Engine version doesn't match. Your version is:")
                                  + " " + engineVersionCheck + ". " + tr("Expecting version:") + " " + ENGINE_VERSION);
             exit(1);
            return false;
        }

        view->show();

        return true;
}

/**
 * Returns the Engine version, if the app using the engine wants to know it:
 */
QString Engine::getEngineVersion()
{
    return ENGINE_VERSION;
}

/**
 * Translate the app based on the language parameter:
 * @return true if there was a translation file.
 */
bool Engine::setAppLanguage(QString language)
{
    /* Sets the translator, so that the app can be translated: */
    if (appTranslator == NULL)
        appTranslator = new QTranslator(0);

    /* Sets the engine translator: */
    if (engineTranslator == NULL)
        engineTranslator = new QTranslator(0);

    qDebug() << "[Engine] App language set to: " << language;

    bool success = true;

    /* Load the engine translations: */
    if (!engineTranslator->load(":/" + QString(TRANSLATION_FOLDER) + "/" + QString(ENGINE_TRANSLATION_NAME) + "_" + language))
        success = false;
    else qApp->installTranslator(engineTranslator);

    /* Load the app translations: */
    if (!appTranslator->load(":/" + QString(TRANSLATION_FOLDER) + "/" + appTranslationName + "_" + language))
        success = false;
    else qApp->installTranslator(appTranslator);

    /* Add the translation as the default translation in the register: */
    QSettings settings;
    settings.setValue(getLanguagePath(), language);

    return success;
}

/** Get the address for the register file (saving language preferences): */
QString Engine::getLanguagePath() {
    return QString(TRANSLATION_FOLDER) + "/defaultLanguage";
}

/** Translate the app based on language chosen before or the operational system's language: */
void Engine::translateAppToDefaultLanguage()
{
    QSettings settings;
    if (settings.contains( getLanguagePath() ))
        setAppLanguage( settings.value( getLanguagePath() ).value<QString>() );
    else
        setAppLanguage( QLocale::system().name() );
}


/**
 * ==== ENGINE FACTORY METHODS ====
 * Encapsulates the factory.
 * Use the methods below to declare new instances of engine components.
 * They are all static methods.
 */

LoadingScreen *Engine::newLoadingScreen(Room *roomView, QWidget *parent)
{
    return engineFactory->newLoadingScreen(roomView, parent);
}

Room *Engine::newRoomView(QWidget *parent)
{
    return engineFactory->newRoomView(parent);
}

QWidget *Engine::newSettingsDialog(UserSettings *settings, QWidget *parent)
{
    return engineFactory->newSettingsDialog(settings, parent);
}

Menu *Engine::newMainMenu(UserSettings *settings, QWidget *parent)
{
    return engineFactory->newMainMenu(settings, parent);
}

User *Engine::newUser(qint32 id, QObject *parent, UserSettings *s, bool isHost)
{
    return engineFactory->newUser(id, parent, s, isHost);
}

UserBox *Engine::newUserBox(User *u, QWidget *parent)
{
    return engineFactory->newUserBox(u, parent);
}

UserSettings *Engine::newUserSettings(QObject *parent, bool isAnotherUser)
{
    return engineFactory->newUserSettings(parent, isAnotherUser);
}

RoomDescription *Engine::newRoomDescription(qint32 port, QString ip, QObject *parent)
{
    return engineFactory->newRoomDescription(port, ip, parent);
}

NewRoom *Engine::newNewRoom(UserSettings *settings, QWidget *parent)
{
    return engineFactory->newNewRoom(settings, parent);
}

FindRoom *Engine::newFindRoom(UserSettings *settings, QWidget *parent)
{
    return engineFactory->newFindRoom(settings, parent);
}

void Engine::closeUpdater()
{
    if (updater == NULL) return;
    updater->close();
}
