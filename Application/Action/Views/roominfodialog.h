#ifndef ROOMINFODIALOG_H
#define ROOMINFODIALOG_H

#include <QDialog>
#include <QDebug>

#include "EngineImpl/Views/roomview.h"

namespace Ui {
class RoomInfoDialog;
}

/**
 * @brief The RoomInfoDialog class is a dialog that displays
 * information about the room, such as room settings.
 */

class RoomInfoDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit RoomInfoDialog(RoomView *room = 0);
    ~RoomInfoDialog();

    /** Called by the dialog controller to display this dialog: */
    void showDialog();

protected:
    /** Retranslates the app if its language changed: */
    void changeEvent(QEvent* event);
    
private slots:
    void on_maxUsersSpinBox_valueChanged(int arg1);

private:

    /**
     * @return the string name of the @param permisson.
     * eg: PERMISSION_HOST_ONLY will return "Host only"
     */
    QString getPermissionString(qint8 permission);

    /** @return the string name of the type based on the enum: */
    QString getTeamSortingTypeString(qint8 type);

    Ui::RoomInfoDialog *ui;
    void refreshFields();

    RoomView *room;
};

#endif // ROOMINFODIALOG_H
