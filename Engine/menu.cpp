#include "menu.h"

QString Menu::news = ""; //Static method

Menu::Menu(UserSettings *settings, QWidget *parent) :
    QWidget(parent)
{
    userSettings = settings;
    if (userSettings == NULL) {
        qWarning() << "Cannot initialize the Main Menu without the user settings.";
        return;
    }
    newRoomDialog = NULL;
    findRoomDialog = NULL;

    /* Clean the temp folder from the update: */
    QString appPath = QCoreApplication::applicationDirPath() + "/" + TEMP_FOLDER;
    cleanUpdateFiles(appPath);
}


void Menu::openNewRoomDialog()
{
    if (!newRoomDialog) {
        newRoomDialog = Engine::newNewRoom(userSettings, this);
        connect(newRoomDialog, SIGNAL(hostRoom(RoomController*)), this, SLOT(joinRoom(RoomController*)));
    }
    newRoomDialog->show();
    newRoomDialog->activateWindow();
}

void Menu::openFindRoomDialog()
{
    if (!findRoomDialog) {
        findRoomDialog = Engine::newFindRoom(userSettings,this);
        connect(findRoomDialog, SIGNAL(joinRoom(RoomController*)), this, SLOT(joinRoom(RoomController*)));
    }
    findRoomDialog->show();
    findRoomDialog->activateWindow();
}

void Menu::joinRoom(RoomController *room)
{
    emit goToRoom(room);
}

NewRoom *Menu::getNewRoomDialog()
{
    return newRoomDialog;
}

FindRoom *Menu::getFindRoomDialog()
{
    return findRoomDialog;
}

void Menu::closeApp()
{
    emit signalClose();
}

void Menu::infoRequested(QStringList list)
{
    /* Received information from the web, such as news */
    QString date = list.at(WebContact::LINE_DATE);
    QString news = list.at(WebContact::LINE_NEWS);
    QString version = list.at(WebContact::LINE_VERSION);
    QString versionLog = list.at(WebContact::LINE_VERSION_LOG);
    QString versionLink = list.at(WebContact::LINE_VERSION_LINK);

    updateNews(date, news);

    /* Check if the app is already updated: */
    if (version != Engine::getAppVersion()) {
        Update *update = new Update(this);

        /* The update class has the power of closing the application: */
        connect(update, SIGNAL(signalClose()), this, SLOT(closeApp()));

        update->requestUpdate(version, versionLog, versionLink);
    }
}

void Menu::timeExpired()
{
    qDebug() << "Requesting web info...";
    emit requestWebInfo();
}

void Menu::updateNews(QString date, QString text)
{
    news = getNews(date, text);
    setNews(news);
}

void Menu::cleanUpdateFiles(QString appPath)
{
    QDir tempDir(appPath);
    if(tempDir.exists())
    {
        Q_FOREACH(QFileInfo info, tempDir.entryInfoList(QDir::NoDotAndDotDot |
                                                        QDir::System | QDir::Hidden  |
                                                        QDir::AllDirs | QDir::Files, QDir::DirsFirst))
        {
            if(info.isDir())
            {
                cleanUpdateFiles(info.absoluteFilePath());
            }
            else
            {
                QFile::remove(info.absoluteFilePath());
            }
        }

        if(!tempDir.rmdir(appPath))
        {
            qDebug() << "[MENU] INITIALIZATION ERROR";
        }
    }
}

void Menu::requestNews()
{
    setNews(news);

    /* Declare a WebContact and set up the contact URL. The WebContact will
       Give the news feed, the location, and tell the MainMenu if the app
       is updated:
     */
    WebContact::getInstance()->setURL(getWebServerURL());

    /* Make the connections with the WebContact and request the info: */
    connect(this, SIGNAL(requestWebInfo()),
            WebContact::getInstance(), SLOT(requestWebInfo()), Qt::QueuedConnection);

    connect(WebContact::getInstance(), SIGNAL(infoRequested(QStringList)),
            this, SLOT(infoRequested(QStringList)), Qt::QueuedConnection);

    /* Set a timeout to emit the signal. There's no need to grab the data so soon: */
    QTimer::singleShot(100, this, SLOT(timeExpired()));
}



QString Menu::getOldNews()
{
    return news;
}
