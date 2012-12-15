#ifndef FINDROOM_H
#define FINDROOM_H

#include <QWidget>
#include <QStringList>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <QInputDialog>
#include "Engine_global.h" /* This class can be used outside the Engine */
#include "webcontact.h"
#include "pingroomtask.h"

/* Make this class "half" aware of the following Engine classes: */
class PingRoomTask;

/**
    @brief This dialog displays the available rooms. It also
    allows the user to connect to a room (returning the
    RoomController of that particular room).
*/

#define DEFAULT_MAX_NUM_OF_CONSECUTIVE_FAILS 10
#define DEFAULT_MAX_NUM_RUNNING_TASKS 10
#define DEFAULT_TIME_WAITING_LIST_SECONDS 5
#define DEFAULT_TOTAL_IPS_TO_FETCH 15

class ENGINESHARED_EXPORT FindRoom : public QDialog
{
    Q_OBJECT
public:
    explicit FindRoom(UserSettings *settings, QWidget *parent = 0);

    enum listColumns {
        COLUMN_PASSWORD,
        COLUMN_USERS,
        COLUMN_ROOM,
        COLUMN_TYPE,
        COLUMN_LATENCY,
        COLUMN_LOCATION,
        COLUMN_HOST,
        COLUMN_HOSTED_SINCE,
        COLUMN_IP,
        COLUMN_PORT,
        TOTAL_COLUMNS
    };

    enum listColumnsSize {
        COLUMN_PASSWORD_SIZE = 26,
        COLUMN_USERS_SIZE = 40,
        COLUMN_ROOM_SIZE = 200,
        COLUMN_TYPE_SIZE = 100,
        COLUMN_LATENCY_SIZE = 55,
        COLUMN_LOCATION_SIZE = 140,
        COLUMN_HOST_SIZE = 60,
        COLUMN_HOSTED_SINCE_SIZE =150
    };
    
signals:
    void joinRoom(RoomController *room);
    void requestRooms(qint32 ipsToFetch);
    
public slots:
    void nicknameChanged(QString name);

private slots:
    void ipListArrived(QStringList list);
    void directConnectionPingResult(RoomDescription *description, PingRoomTask *task);
    void oneListConnectionPingResult(RoomDescription *description, PingRoomTask *task);
    void roomSelected(int row, int column);
    void timeExpired();

protected:
    void setupRoomListHeaderLabels();
    void setMaxNumberOfConsecutiveFails(int max);
    void setMaxNumberOfRunningTasks(int max);
    void setSecondsWaitingTime(int seconds);
    void setTotalIpsToFetch(int total);
    void pingIpServer(QString ip, int port);
    void connectToIpServer(QString ip, int port);
    void setupFindRoomUI();
    UserSettings* getUserSettings();
    void openUserSettingsDialog();
    void joinSelectedRoom();
    void refreshRoomList();
    void refreshUserNicknameLabel();
    void stopSearch();
    virtual void linkViewComponents(QPushButton *&stopSearchBtn, QPushButton *&joinRoomBtn,   QPushButton *&joinIpServerBtn,
                                    QPushButton *&settingsBtn,   QPushButton *&refreshListBtn, QPushButton *&pingIpServerBtn,
                                    QLabel *&searchRoomInfoLb,   QLabel *&listTitleLb,         QLabel *&nicknameLb,
                                    QTableWidget *&listTable) = 0;

private:
    void ping(QString ip, int port);
    void pingSomeRooms();
    void createRoomsList();
    void declareNullComponents();
    void connectToRoom(QString ip, int port, bool passwordProtected);
    void setListTitle(int rooms, bool searching = false);
    QString getPassword();
    void removeAllRunningTasks();
    void setUpColumnSizes();
    void addTableRow(RoomDescription *description);
    QString getListDetail(RoomDescription *description);

    QStringList listHeader;



    QStringList waitingRoomsToPing;

    QTimer searchTimer;
    QStringList listDetails; //ips from rooms that were found
    int roomsFound;
    int roomsPinged;
    int consecutivePingFails;
    int maxConsecutiveFails;
    int maxRunningTasks;
    int totalIpsToFecth;
    int secondsWaitingTime;
    bool searchedRoomIsPasswordProtected;
    bool serverResponded;

    UserSettings *userSettings;
    QList<PingRoomTask*> runningTasks;

    QPushButton *stopSearchBtn;
    QPushButton *joinRoomBtn;
    QPushButton *joinIpServerBtn;
    QPushButton *settingsBtn;
    QPushButton *refreshListBtn;
    QPushButton *pingIpServerBtn;

    QLabel *searchRoomInfoLb;
    QLabel *listTitleLb;
    QLabel *nicknameLb;
    QTableWidget *listTable;
};

#endif // FINDROOM_H
