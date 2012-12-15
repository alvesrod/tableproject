#include "roominfo.h"

RoomInfo::RoomInfo(qint32 port, QString ip, QObject *parent) :
    RoomDescription(port, ip, parent)
{
    table = new Table(this); /* Deleted by RoomView (Qt deletes it) */

    lockObjectPermission = PERMISSION_EVERYONE;
    hideObjectPermission = PERMISSION_EVERYONE;
    addObjectPermission = PERMISSION_EVERYONE;
    changeTeamsPermission = PERMISSION_EVERYONE;
    removeObjectPermission = PERMISSION_EVERYONE;
}

RoomInfo::~RoomInfo()
{
}

void RoomInfo::setTable(Table *table) { this->table = table; }
Table *RoomInfo::getTable() { return table; }

qint8 RoomInfo::getLockObjectPermission() { return lockObjectPermission; }
void RoomInfo::setLockObjectPermission(qint8 permission) { lockObjectPermission = permission; }

qint8 RoomInfo::getHideObjectPermission() { return hideObjectPermission; }
void RoomInfo::setHideObjectPermission(qint8 permission) { hideObjectPermission = permission; }

qint8 RoomInfo::getAddObjectPermission() { return addObjectPermission; }
void RoomInfo::setAddObjectPermission(qint8 permission) { addObjectPermission = permission; }

qint8 RoomInfo::getRemoveObjectPermission() { return removeObjectPermission; }
void RoomInfo::setRemoveObjectPermission(qint8 permission) { removeObjectPermission = permission; }

qint8 RoomInfo::getChangeTeamsPermission() { return changeTeamsPermission; }
void RoomInfo::setChangeTeamsPermission(qint8 permission) { changeTeamsPermission = permission; }

qint8 RoomInfo::getTeamSortingType() { return teamSortingType; }
void RoomInfo::setTeamSortingType(qint8 type) { teamSortingType = type; }


QString RoomInfo::getRoomFolderPath()
{
    return qApp->applicationDirPath() + "/" + ROOM_FOLDER_NAME + "/" + getRoomType() + "/";
}

/** Method used to send variables to others through the network: */
void RoomInfo::addDataToStream(QDataStream &stream)
{
    stream << lockObjectPermission;
    stream << hideObjectPermission;
    stream << addObjectPermission;
    stream << changeTeamsPermission;
    stream << removeObjectPermission;
    stream << teamSortingType;
    stream << table;
}

/** Method used to receive variables from others through the network: */
void RoomInfo::useDataFromStream(QDataStream &stream)
{
    stream >> lockObjectPermission;
    stream >> hideObjectPermission;
    stream >> addObjectPermission;
    stream >> changeTeamsPermission;
    stream >> removeObjectPermission;
    stream >> teamSortingType;

    /* It's important to create a new table or the items from the previous one will be here. */
    table = new Table(this); //Qt will delete the old one later since we are passing "this" to a QObject
    stream >> table;
}

bool RoomInfo::autosaveTableFile()
{
    return saveRoomFile(getRoomFolderPath() + getRoomType()
                        + AUTOMATIC_SAVED_TABLE_FILE_NAME + "." + TABLE_FILE_TYPE);
}

bool RoomInfo::saveAsTableFile(QWidget *view)
{
    /* Create all the folders necessary if they don't exist: */
    QDir dir;
    dir.mkpath(getRoomFolderPath());

    QString fileName = QFileDialog::getSaveFileName(view, tr("Save table"),
                                                    getRoomFolderPath() + "/" + getRoomType(),
                                                    tr("Table Files ") + "(*." + TABLE_FILE_TYPE + ")");
    if (fileName.isEmpty())
        return false;
    else {
        if (!saveRoomFile(fileName)) {
            QMessageBox::warning(view, tr("Save Table"),
                        tr("The application could not save the file."));
            return false;
        } else {
            if (!fileName.contains(getRoomFolderPath(), Qt::CaseInsensitive)) {
                QMessageBox::warning(view, tr("Save Table"),
                    + "<b>" + tr("You saved the table file outside its main folder.") + "</b><br>"
                    + tr("The file was saved, but remember that this file might use images from the")
                    + " <i>" + getRoomType() + "</i> " + tr("folder.") );
            }
        }
    }
    return true;
}
