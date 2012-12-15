#ifndef NEWROOMDIALOG_H
#define NEWROOMDIALOG_H

#include <QDialog>
#include <QDebug>
#include "../Engine/newroom.h"
#include "../roominfo.h"
#include "constants.h"
#include "../membersettings.h"

namespace Ui {
class NewRoomDialog;
}

/**
 * The NewRoomDialog is a dialog that allows the user to host a new room.
 * (Check the form of this class in the Forms folder)
 */

class NewRoomDialog : public NewRoom
{
    Q_OBJECT
    
public:
    NewRoomDialog(UserSettings *settings, QWidget *parent = 0);
    ~NewRoomDialog();

    /** Change the type of room (it also help test cases): */
    void setRoomTypeField(QString type);

protected:

    /** Allow the superclass to know the components that will display the elements of the view. */
    void linkViewComponents(QLabel *&nicknameLb);

    /** Retranslates the app if its language changed: */
    void changeEvent(QEvent *event);
    
private slots:
    /** Called to display in the view the variables in the RoomInfo object. */
    void tableDataUpdated();

    /** Button clicks: */
    void on_startServerBtn_clicked();
    void on_settingsBtn_clicked();
    void on_loadTableBtn_clicked();

    void on_portCheckBox_toggled(bool checked);

private:
    /** Helper to prepare the RoomInfo object based on the values in the view: */
    void prepareRoomDescription();

    /* This variable stores all the configurations of the room that are decided by the user: */
    RoomInfo *roomDescription;

    Ui::NewRoomDialog *ui;

};

#endif // NEWROOMDIALOG_H
