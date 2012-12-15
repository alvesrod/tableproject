#ifndef ROOMINFO_H
#define ROOMINFO_H

#include <QString>

#include "../../Engine/roomdescription.h"
#include "Action/Views/table.h"

class Table;

/**
 * This class stores every specific information
 * about the room. This information is shared
 * in the loading screen. So that, every user in
 * the room knows about it. This information can
 * also be shared through save/load room files.
 */

/* Name of the folder that stores the subfolders with the room properties: */
#define ROOM_FOLDER_NAME "Tables"
#define AUTOMATIC_SAVED_TABLE_FILE_NAME "_autosaved"

class RoomInfo : public RoomDescription
{
public:
    RoomInfo(qint32 port, QString ip = LOCAL_IP, QObject *parent = 0);
    ~RoomInfo();

    enum TeamSortingType {
        TEAM_ONE_PER_TEAM,
        TEAM_RANDOM_TEAMS,
        TEAM_SAME_TEAM
    };

    /* Getters and setters for the room configurations: */

    void setTable(Table *table);
    Table* getTable();

    qint8 getLockObjectPermission();
    void setLockObjectPermission(qint8 permission);

    qint8 getRemoveObjectPermission();
    void setRemoveObjectPermission(qint8 permission);

    qint8 getHideObjectPermission();
    void setHideObjectPermission(qint8 permission);

    qint8 getAddObjectPermission();
    void setAddObjectPermission(qint8 permission);

    qint8 getChangeTeamsPermission();
    void setChangeTeamsPermission(qint8 permission);

    qint8 getTeamSortingType();
    void setTeamSortingType(qint8 type);

    /** Returns the full path to the current room folder: */
    QString getRoomFolderPath();

    /**
     * Save the table in a location decided by the user in runtime:
     * This will open a dialog asking to save an image.
     * @param view: the view that the dialog will be in.
     */
    bool saveAsTableFile(QWidget *view);

    /** Save the table in a file already specified (by the application): */
    bool autosaveTableFile();

private:
    void addDataToStream(QDataStream &stream);
    void useDataFromStream(QDataStream &stream);

    Table *table;

    qint8 lockObjectPermission;
    qint8 hideObjectPermission;
    qint8 addObjectPermission;
    qint8 changeTeamsPermission;
    qint8 removeObjectPermission;
    qint8 teamSortingType;
};

#endif // ROOMINFO_H
