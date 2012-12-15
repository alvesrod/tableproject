#ifndef ROOMMEMBER_H
#define ROOMMEMBER_H

#include <QObject>
#include <QTime>
#include <QDebug>
#include <QStringList>
#include "../../Engine/usersettings.h"
#include "../../Engine/packagehandlerinterface.h"
#include "../../Engine/user.h"

/**
 * This class inherits User and stores
 * specific information about the user
 * that is currently in the room. Every
 * app-specific thing you need to know about
 * a particular user online in the room, you
 * may find here (such as the user team). It's
 * good to remember that the User class also
 * has a User Settings object. So, some info
 * about the user might be there. You can just
 * call the public method getUserSettings() to
 * get that extra info. You can also access from
 * here the values from any class that extends
 * User Settings, such as MemberSettings.
 */

class RoomMember : public User
{
    Q_OBJECT

public:

    enum TeamColor {
        TEAM_RED,
        TEAM_BLUE,
        TEAM_WHITE,
        TEAM_BLACK,
        TEAM_GREEN,
        TEAM_YELLOW,
        TEAM_GRAY,
        TEAM_PINK,
        TEAM_PURPLE,
        TEAM_ORANGE,
        TEAM_OTHERS,
        TOTAL_OF_TEAMS //This one is just to get the size of the enum
    }; //The string value of the teams above is in the getTeamList() method.

    /** Types of messages that onMessageReceived() and onMessageRequest() can handle: */
    enum MemberMessageTypes {
       /*  0 */ MSG_CHANGE_TEAM
    };

    RoomMember(qint32 id =INVALID_ID, QObject *parent = 0, UserSettings *settings = 0, bool isUserHost = false);
    TeamColor getTeam();
    QString getTeamString();
    void setTeam(TeamColor color);
    static QStringList getTeamList();

    /**
     * Assign the user to an empty team (or last team if all full).
     * @return false if all teams are full.
     */
    bool assignToEmptyTeam(QList<User*> users);

signals:
    void teamChanged(RoomMember* memberChanged, RoomMember* memberWhoChanged);
    
public slots:

private:
    /* Helper to display a message in the chat saying that the user changed teams: */
    void addTeamUpdateChatMessage(RoomMember *userWhoChanged);

    /* Implementations of the User class to deal with network messages: */
    void onMessageReceived(QDataStream &stream, qint16 type, User *editor);
    void onMessageRequest(QDataStream &stream, qint16 type);
    void addDataToStream(QDataStream &stream);
    void useDataFromStream(QDataStream &stream);

    qint8 team;
    bool firstTeamAssignment; //true when it was the firts time changing teams.
};

#endif // ROOMMEMBER_H
