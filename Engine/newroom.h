#ifndef NEWROOM_H
#define NEWROOM_H

#include <QWidget>
#include <QLabel>
#include "Engine_global.h" /* This class can be used outside the Engine */
#include "roomdescription.h"

/* Make this class "half" aware of the following Engine classes: */
class RoomController;

/**
 * @brief The NewRoom class is the dialog where the user can set
 * the properties of a new room (such as room name and password)
 * in order to host it. When the user wants to host a room, the
 * information about the room is going to be sent to the main
 * menu. So, this class belongs to the main menu.
 */

class ENGINESHARED_EXPORT NewRoom : public QDialog
{
    Q_OBJECT

public:
    explicit NewRoom(UserSettings *settings, QWidget *parent = 0);
    
signals:
    void hostRoom(RoomController *room);

protected:
    virtual void linkViewComponents(QLabel *&nicknameLb) = 0;
    void openUserSettingsDialog();
    void refreshUserNicknameLabel();
    void setupNewRoomUI();
    void startNewRoom(RoomDescription *roomDescription);
    QString getUserLocation();

private slots:
    void nicknameChanged(QString name);

private:
    UserSettings *userSettings;
    QLabel *nicknameLb;
    
};

#endif // NEWROOM_H
