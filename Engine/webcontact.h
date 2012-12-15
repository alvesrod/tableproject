#ifndef WEBCONTACT_H
#define WEBCONTACT_H

#include <QThread>
#include <QStringList>
#include <QDebug>
#include <QMutex>
#include <QTimer>
#include <QNetworkReply>

#include "engine.h"

#define URL_CONNECT_FILENAME "connect.php" //The filename in the website where the interaction webserver-app happens
#define SQL_TYPE_PARAMETER "a"
#define SQL_PORT_PARAMETER "p"
#define SQL_KEY_PARAMETER "k"

/* Refresh colddowns. A request before the colddown is over will just be ignored: */
#define REFRESH_ROOMS_BREAK_MILLISECONDS 4000 //The colddown between room list requests.
#define REFRESH_INFO_REQUEST_BREAK_MILLISECONDS 600000 //The colddown between info requests.

/**
  @brief
  This class contacts the web servers to grab information, such as location,
  list of ips, add a user to the ip list, get news and version, etc.

  ==== THIS CLASS IS A SINGLETON ====
  To use it call WebContact::getInstance();

  */

class WebContact : public QThread
{
    Q_OBJECT
public:

    enum mySQLMessageTypes {
        SQL_REMOVE_IP,
        SQL_ADD_IP,
        SQL_INFO,
        SQL_GET_IPS
    };

    enum mySQLMessageAnswers {
        SQL_ANSWER_ROOM_CLOSED,
        SQL_ANSWER_FAIL,
        SQL_ANWER_LIST_FETCHED,
        SQL_ANSWER_INFO,
        SQL_ANSWER_ROOM_PUBLISHED
    };

    enum infoLineOrder {
        LINE_DATE,
        LINE_NEWS,
        LINE_VERSION,
        LINE_VERSION_LOG,
        LINE_VERSION_LINK,
        LINE_IP,
        LINE_LOCATION,
        LINE_TOTAL_LINES
    };

    /** Grabs an instance of this singleton. Create one if this class was not declared: */
    static WebContact* getInstance();

    /** Set the URL where the webserver files are stored. This should be set before requesting info: */
    void setURL(QString url);

protected:
    /** Necessary since this class is a thread: */
    void run();

signals:
    /**
     * @brief roomListRequested signal is sent when the application got the
     * list of rooms.
     * @param list is the list of ips and ports.
     */
    void roomListRequested(QStringList list);

    /**
     * @brief infoRequested signal is sent when the application got the
     * basic information from the webserver. This basic information includes
     * the app version, the ip, the app news, etc.
     * @param list is a list of all the information. Use enum infoLineOrder to
     * know which information is stored in which line.
     */
    void infoRequested(QStringList list);

    /**
     * @brief locationReceived signal is sent when the application knows the
     * user geolocation.
     * @param location is a formatted string of the location received.
     */
    void locationReceived(QString location);

    /**
     * @brief ipReceived signal is sent when the application knows the user ip.
     */
    void ipReceived(QString ip);

    /**
     * @brief the webserver confirmed that the room was published on the list.
     */
    void roomPublished();
    
public slots:

    /**
     * Request the list of all ips.
     * A signal infoRequested() is called when the list arrives:
     */
    void requestPublishedRooms(qint32 ipsToFetch = 10);

    /** Add the user ip and port to the list of ips online, so others can join a room: */
    void publishRoom(qint32 port);

    /** Remove the user ip and port from the list of ips online: */
    void removePublishedRoom(qint32 port);

    /** Get the news, version, the address to where the new version is stored, your ip and location: */
    void requestWebInfo();


    /* Don't worry about private methods below: */

private slots:
    void replyFinished(QNetworkReply* reply);
    void timeExpiredRoomRefresh();
    void timeExpiredInfoRefresh();

private:
    WebContact();
    void makeListRequest(QUrl params);
    QString generateHashKey(int value);
    QNetworkRequest getRequest();

    static WebContact *singleton;
    QNetworkAccessManager *manager;
    QMutex urlLock;
    QString contactURL;
    bool isWaitingRoomsArrive;
    bool requestedInfo;
};

#endif // WEBCONTACT_H
