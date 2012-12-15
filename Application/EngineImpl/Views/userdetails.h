#ifndef USERDETAILS_H
#define USERDETAILS_H

#include <QWidget>
#include <QDebug>
#include <QMessageBox>
#include "../roommember.h"
#include "../roominfo.h"
#include "../Engine/userbox.h"

namespace Ui {
class UserDetails;
}

/**
 * UserDetails is the view that displays the details of a specific user. You can
 * access this view through the list of users in the chat. If you are
 * unsure about which class is this, check its Form in the Forms folder.
 */

class UserDetails : public UserBox
{
    Q_OBJECT
    
public:
    UserDetails(User *user, QWidget *parent = 0);
    ~UserDetails();

protected:
    /* Retranslates the app if its language changed: */
    void changeEvent(QEvent *event);

    /* Called when there's a new user latency: */
    void displayLatency(int latency);

    /* Called when the user left, but the userbox is still opened: */
    void on_user_left();

    /* Called to refresh the UI and reset all permissions (usually after changes): */
    void refreshUI();

    /** Below is just private helpers: */
private slots:
    void on_teamComboBox_currentIndexChanged(int index);
    void on_isAdminCheckBox_clicked(bool checked);
    void teamUpdateFromUser(RoomMember *userChanged, RoomMember*userWhoChanged);

private:
    void setUpTeamComboBox();
    void setUpPermissions();
    bool getChangeTeamPermission();
    void changeTeam(int team, bool submitChanges = true);
    void updateOnlineLb();

    Ui::UserDetails *ui;
    RoomMember *member; //The user whose details are being shown.
};

#endif // USERDETAILS_H
