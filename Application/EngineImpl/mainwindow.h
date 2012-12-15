#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/* Qt libraries: */
#include <QMainWindow>
#include <QDockWidget>
#include <QPalette>
#include <QLabel>

/* Important constants that might be used: */
#include "constants.h"

/* Views that the MainWindow will only open: */
#include "Views/roomview.h"
#include "Views/settingsdialog.h"

/* Class that the MainWindow inherits: */
#include "../../Engine/viewcontroller.h"

/**
  This class inherits ViewController, which is an Engine class that does
  the switching of views. So, the job of this class is call the menu,
  to open extra views, to control the Menu bar and Status Bar, and
  to do what is left in the switching of views.

  The private methods 'on_mainMenu_loaded', 'on_loadingScreen_loaded' and 'on_roomView_loaded'
  are called when the views are about to be loaded. So that, this class can set the
  extra connections to the views (if there is any).

  This class is allowed to know about the app's view because it is outside the Engine.
 */

namespace Ui {
class MainWindow;
}

class MainWindow : public ViewController
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent);
    ~MainWindow();

    /* View Getters: */
    QWidget* getMenuWidget();
    QWidget* getRoomViewWidget();

protected:
    /* When the user closes the app: */
    void closeEvent(QCloseEvent *event);

    /* When the user changes the app language: */
    void changeEvent(QEvent *event);

protected slots:

    /* Slot triggered by the Room asking to enable/disable a button: */
    void buttonSetEnabled(int button, bool enabled);

private slots:

    /* Slots triggered by buttons in the menu bar: */

    void on_exitAction_triggered();

    void on_joinRoomAction_triggered();

    void on_hostRoomAction_triggered();

    void on_leaveRoomAction_triggered();

    void on_actionSettings_triggered();

    void on_actionEnglishTr_triggered();

    void on_actionPortugueseTr_triggered();

    void on_actionChineseTr_triggered();

    void on_throwDiceAction_triggered();

    void on_addObjectAction_triggered();

    void on_actionZoom_In_triggered();

    void on_actionZoom_Out_triggered();

    void on_actionRotate_Left_triggered();

    void on_actionRotate_Right_triggered();

    void on_actionSave_Table_triggered();

    void on_actionRoom_Info_triggered();

private:

    /* Called when the Main Menu was opened: */
    void on_mainMenu_loaded();

    /* Called when the Loading Screen was opened: */
    void on_loadingScreen_loaded();

    /* Called when the Room was opened: */
    void on_roomView_loaded();

    /* Change the app language when called: */
    void changeAppLanguage(QString languageCode, QAction *languageAction);

    /* Called when a language is changed in the menu bar: */
    void uncheckAllLanguages();

    Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
