#ifndef UserButtonBox_H
#define UserButtonBox_H

#include <QWidget>
#include <QSound>
#include "../user.h"

namespace Ui {
class UserButtonBox;
}

/**
 * @brief The UserButtonBox class is the inner view in the
 * list of users online that allows you to ban/chat/edit a user.
 * This class cannot be accessed outside the Engine.
 */

class UserButtonBox : public QWidget
{
    Q_OBJECT
    
public:
    explicit UserButtonBox(User *boxUser, User *currentUser, RoomDescription *description, QWidget *parent = 0);
    ~UserButtonBox();

    /** Get the user that represents this inner view: */
    User* getUser();

    /** Enable or disable the "ban" button based on whether or not you have permission to ban this user: */
    void updateBanButtonPermission();

signals:
    void privateMsgBtnClicked(User *user); //You want to talk privately with this @param user
    void moreBtnClicked(User *user); //You want to see more about this @param user
    void banUserBtnClicked(User *user); //You want to ban this @param user.

protected:

    /** Retranslates the app if its language changed: */
    void changeEvent(QEvent* event);
    
private slots:

    /* Button clicks: */

    void on_privateMessageBtn_clicked();

    void on_moreBtn_clicked();

    void on_banBtn_clicked();

private:
    bool getBanPermission();
    void banUser();

    Ui::UserButtonBox *ui;
    RoomDescription *roomDescription; //Description is required to know if you have permission to ban.

    User *currentUser; //That user represents you (who are seeing this view)
    User *user;       //That user represents the user from this view.

};

#endif // UserButtonBox_H
