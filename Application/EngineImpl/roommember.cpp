#include "roommember.h"

RoomMember::RoomMember(qint32 id, QObject *parent, UserSettings *settings, bool isUserHost):
    User(id, parent, settings, isUserHost)
{
    team = TEAM_OTHERS;
    firstTeamAssignment = true;
}

QStringList RoomMember::getTeamList()
{
    return tr("Red, Blue, White, Black, Green, Yellow, Gray, Pink, Purple, Orange, Others").split(", ");
}

bool RoomMember::assignToEmptyTeam(QList<User *> users)
{
    bool foundTeam;

    for (int i = 0; i < TOTAL_OF_TEAMS; i++) {
        foundTeam = true;
        foreach (User *user, users) {
            RoomMember *member = dynamic_cast<RoomMember*>(user);
            if (member == NULL) continue;
            if (member->getTeam() == i) {
                foundTeam = false;
                break;
            }
        }
        if (foundTeam) {
            setTeam((TeamColor) i);
            return true;
        }
    }

    if (!foundTeam) setTeam(TEAM_OTHERS);
    return false;
}

RoomMember::TeamColor RoomMember::getTeam()
{
    return (TeamColor) team;
}

QString RoomMember::getTeamString()
{
    return getTeamList().at(team);
}

void RoomMember::setTeam(RoomMember::TeamColor color)
{
    team = color;
}

void RoomMember::onMessageReceived(QDataStream &stream, qint16 type, User *editor)
{
    /*
     * Convert from User to RoomMember since this is the inheritance we are using
     * in the Engine Factory:
     */
    RoomMember* memberEditor = dynamic_cast<RoomMember*> (editor);

    /* Go through all message types: */
    switch (type) {

    case MSG_CHANGE_TEAM:
        stream >> team;

        /* Chat messages only after the first automatic assignment: */
        if (!firstTeamAssignment)
            addTeamUpdateChatMessage(memberEditor);
        else firstTeamAssignment = false;

        emit teamChanged(this, memberEditor);
        break;

    default :
        qWarning() << "Received an invalid member message type" << type;
    }
}

/**
 * Subtypes of message that can be sent through the network when
 * an object sends a message of type EDIT_USER.
 */
void RoomMember::onMessageRequest(QDataStream &stream, qint16 type)
{
    switch (type) {

    case MSG_CHANGE_TEAM:
        stream << team;
        break;

        /* You can add more cases here for more types of message received */

    default :
        qWarning() << "[RoomMember] Received an invalid member message type" << type;
    }

}

void RoomMember::addDataToStream(QDataStream &stream)
{
    /* Variables that will be shared through the network to other users: */
    stream << team;
    stream << firstTeamAssignment;
}

void RoomMember::useDataFromStream(QDataStream &stream)
{
    /* Variables that will be shared through the network to other users: */
    stream >> team;
    stream >> firstTeamAssignment;
}

/** Add a chat message saying that somebody changed teams: */
void RoomMember::addTeamUpdateChatMessage(RoomMember *userWhoChanged)
{
    QString chatMsg = "<b>";
    if (this == userWhoChanged)
        chatMsg.append(getNickname() + "</b> " + tr("changed team to") + " <b>" + getTeamString());
    else
        chatMsg.append(userWhoChanged->getNickname()
            + "</b> " + tr("changed the team of") + " <b>" + getNickname() + "</b> "+ tr("to") + " <b>"
            + getTeamString());
    chatMsg.append("</b>");
    addChatMessage(chatMsg, QColor("#9385B6"));
}
