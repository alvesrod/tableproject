#ifndef FINDROOMDIALOG_H
#define FINDROOMDIALOG_H

#include <QDialog>

#include "../Engine/roomdescription.h"
#include "../Engine/usersettings.h"
#include "../Engine/webcontact.h"
#include "../Engine/findroom.h"

#include "constants.h"

/**
 * This class is the dialog where users search for opened rooms.
 * It tells the FindRoom Engine superclass about the components in the view.
 */

namespace Ui {
class FindRoomDialog;
}

class FindRoomDialog : public FindRoom
{
    Q_OBJECT
    
public:
    FindRoomDialog(UserSettings *settings, QWidget *parent = 0);
    ~FindRoomDialog();

protected:
    /** Allow the superclass to know the components that will display the elements of the view. */
    void linkViewComponents(QPushButton *&stopSearchBtn, QPushButton *&joinRoomBtn,   QPushButton *&joinIpServerBtn,
                            QPushButton *&settingsBtn,   QPushButton *&refreshListBtn, QPushButton *&pingIpServerBtn,
                            QLabel *&searchRoomInfoLb,   QLabel *&listTitleLb,         QLabel *&nicknameLb,
                            QTableWidget *&listTable);

    /** Retranslates the app if its language changed: */
    void changeEvent(QEvent *event);

private slots:
    /** Button clicks: */
    void on_refreshListBtn_clicked();
    void on_joinRoomListBtn_clicked();
    void on_settingsBtn_clicked();
    void on_joinServerBtn_clicked();
    void on_pingBtn_clicked();
    void on_stopSearchBtn_clicked();

private:
    Ui::FindRoomDialog *ui;
};

#endif // FINDROOMDIALOG_H
