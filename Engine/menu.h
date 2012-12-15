#ifndef MENU_H
#define MENU_H

#include <QWidget>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include "Engine_global.h" /* This class can be used outside the Engine */
#include "newroom.h"
#include "findroom.h"
#include "update.h"

/* Make this class "half" aware of the following Engine classes: */
class RoomController;

/**
 * @brief The Menu class is the engine behind the main menu view
 * where users can open dialogs to join or host rooms. The view
 * is going to extend this class. This is also where the user
 * can read the application news and where the app checks for
 * new versions. So, the main menu is supposed to be the first
 * view loaded when the app starts.
 */

class ENGINESHARED_EXPORT Menu : public QWidget
{
    Q_OBJECT
public:
    explicit Menu(UserSettings *settings, QWidget *parent = 0);
    
signals:
    void goToRoom(RoomController *room); ///send a signal asking to join a room
    void signalClose(); //send a signal asking to close the application
    void requestWebInfo(); //request web info, such as location, ip, news, etc.

protected:
    /** Retrun the current value of the news variable: */
    QString getOldNews();

    /** Those classes are going to be implemented outside the Engine: */
    virtual QString getWebServerURL() = 0;
    virtual QString getNews(QString date, QString text) = 0;
    virtual void setNews(QString news) = 0;

    /** Called to contact the web server and request all the basic data: */
    void requestNews();

protected slots:
    void openFindRoomDialog();
    void openNewRoomDialog();
    void closeApp();

private slots:
    void infoRequested(QStringList list); //The info requested throught the "requestWebInfo() signal"
    void timeExpired();
    void joinRoom(RoomController *room);

    NewRoom *getNewRoomDialog();
    FindRoom *getFindRoomDialog();

private:
    void updateNews(QString date, QString text);

    /** Clean any folder created by the update (in case the app was updated): */
    QString appPath;
    void cleanUpdateFiles(QString appPath);
    NewRoom *newRoomDialog;
    FindRoom *findRoomDialog;
    UserSettings *userSettings;
    static QString news;
};

#endif // MENU_H
