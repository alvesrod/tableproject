#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    ViewController(parent),
    ui(new Ui::MainWindow)
{
	/*
	 * Initialize and display the GUI to the user:
	 */
    ui->setupUi(this);

    /*
     * Set the background color to white:
     */
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setPalette(pal);

    /*
     * Set the window title to be the application name:
     */
    setWindowTitle(APP_NAME);

    /*
     * Display the Main Menu in the app screen:
     */
    openMainMenu();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/** 
 * When the "Settings" button in the menu bar is pressed,
 * open the settings dialog:
 */
void MainWindow::on_actionSettings_triggered()
{
    openSettingsDialog();
}

void MainWindow::on_mainMenu_loaded()
{
    /*
     * Disable the room menu because the user is not in a room.
     * This is an action associated with the menu in the top of the window:
     */
    ui->menuRoom->setDisabled(true);

    /* Enable a user to host or join a room since he is not in any room: */
    ui->hostRoomAction->setDisabled(false);
    ui->joinRoomAction->setDisabled(false);

    /* Do not allow the user to leave a room when he is not in one: */
    ui->leaveRoomAction->setDisabled(true);

    /* Do not allow the user to save a table when he is not in a room: */
    ui->actionSave_Table->setDisabled(true);
}

void MainWindow::on_loadingScreen_loaded()
{
    ui->hostRoomAction->setDisabled(true); //Disable the host room button in the main menu.
    ui->joinRoomAction->setDisabled(true); //Disable the find room button in the main menu.
}

void MainWindow::on_roomView_loaded()
{
    ui->menuRoom->setDisabled(false); //Enable the room menu in the menu bar.
    ui->leaveRoomAction->setDisabled(false); //Allow the user to leave the room.
    ui->actionSave_Table->setDisabled(false); //Allow the user to save the table.
}

/**
 * Action triggered when the "Join Room"
 * button is pressed in the menu bar:
 */
void MainWindow::on_joinRoomAction_triggered()
{
    emit joinRoomMenuBtnClicked(); //The Main Menu will receive the signal.
}

/**
 * Action triggered when the "Host Room"
 * button is pressed in the menu bar:
 */
void MainWindow::on_hostRoomAction_triggered()
{
    emit hostRoomMenuBtnClicked(); //The Main Menu will receive the signal.
}

/**
 * Action triggered when the "Leave Room"
 * button is pressed in the menu bar:
 */
void MainWindow::on_leaveRoomAction_triggered()
{
    emit leaveRoomMenuBtnClicked(); //The Room View will receive the signal.
}

/**
 * Action triggered when the "Save Table"
 * button is pressed in the menu bar:
 */
void MainWindow::on_actionSave_Table_triggered()
{
    sendRoomButtonClickedSignal(RoomView::BTN_SAVE_TABLE);
}

/**
 * When the "Exit" button in the menu bar is pressed, close the app:
 */
void MainWindow::on_exitAction_triggered()
{
    close();
}

/**
 * When the "Throw Dice" button in the menu bar is pressed, warn the Room:
 */
void MainWindow::on_throwDiceAction_triggered()
{
    sendRoomButtonClickedSignal(RoomView::BTN_THROW_DICE);
}


/**
 * When the "Add Object" button in the menu bar is pressed.
 * The Room View is going to handle that since this is
 * part of what the application is all about.
 */
void MainWindow::on_addObjectAction_triggered()
{
    sendRoomButtonClickedSignal(RoomView::BTN_ADD_OBJECT);
}

/** Zoom In button in the menu bar pressed: */
void MainWindow::on_actionZoom_In_triggered()
{
    sendRoomButtonClickedSignal(RoomView::BTN_ZOOM_IN);
}

/** Zoom Out button in the menu bar pressed: */
void MainWindow::on_actionZoom_Out_triggered()
{
    sendRoomButtonClickedSignal(RoomView::BTN_ZOOM_OUT);
}

/** Room Info button in the menu bar pressed: */
void MainWindow::on_actionRoom_Info_triggered()
{
    sendRoomButtonClickedSignal(RoomView::BTN_ROOM_INFO);
}

void MainWindow::on_actionRotate_Left_triggered()
{
    sendRoomButtonClickedSignal(RoomView::BTN_ROTATE_LEFT);
}

void MainWindow::on_actionRotate_Right_triggered()
{
    sendRoomButtonClickedSignal(RoomView::BTN_ROTATE_RIGHT);
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    /* Main menu has no confirmation dialog: */
    if (centralWidget() == getMainMenu()) return;

    /* Ask if the user really wants to leave the app: */
    int reply = QMessageBox::question(this, tr("Quit"), tr("Are you sure you want to leave?"),
                                                              QMessageBox::Yes, QMessageBox::No);
    if (reply == QMessageBox::Yes)
        event->accept();
    else event->ignore();
}

QWidget *MainWindow::getMenuWidget()
{
    return getMainMenu();
}

QWidget *MainWindow::getRoomViewWidget()
{
    return getRoomView();
}

/** Change the app language (called by the action_triggered methods): */
void MainWindow::changeAppLanguage(QString languageCode, QAction *languageAction)
{
    /* English is the default language, so it might not have a file for it. */
    if ( (Engine::setAppLanguage(languageCode)) || (languageCode == "en") ) {
        uncheckAllLanguages();
        languageAction->setChecked(true);
    }
    else
        qWarning() << "[MainWindow] Error: Language file might be missing for language " << languageCode;
}

/** Unchecks all the languages in the language menu: */
void MainWindow::uncheckAllLanguages()
{
    ui->actionPortugueseTr->setChecked(false);
    ui->actionEnglishTr->setChecked(false);
    ui->actionChineseTr->setChecked(false);
}

/** Retranslates the app if its language changed: */
void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
    QMainWindow::changeEvent(event);
}

/** Enable or disable a button from the menu bar based on a request from the Room: */
void MainWindow::buttonSetEnabled(int button, bool enabled)
{
    switch(button) {

    case RoomView::BTN_ADD_OBJECT:
        ui->addObjectAction->setEnabled(enabled);
        break;

    case RoomView::BTN_ROTATE_LEFT:
        ui->actionRotate_Left->setEnabled(enabled);
        break;

    case RoomView::BTN_ROTATE_RIGHT:
        ui->actionRotate_Right->setEnabled(enabled);
        break;

    case RoomView::BTN_SAVE_TABLE:
        ui->actionSave_Table->setEnabled(enabled);
        break;

    case RoomView::BTN_THROW_DICE:
        ui->throwDiceAction->setEnabled(enabled);
        break;

    case RoomView::BTN_ZOOM_IN:
        ui->actionZoom_In->setEnabled(enabled);
        break;

    case RoomView::BTN_ZOOM_OUT:
        ui->actionZoom_Out->setEnabled(enabled);
        break;

    default :
        qWarning() << "[MainWindow] Received invalid button (number " << button << ").";
    }
}

/** Change the app language to English: */
void MainWindow::on_actionEnglishTr_triggered()
{
    changeAppLanguage( "en" , ui->actionEnglishTr );
}

/** Change the app language to Portuguese: */
void MainWindow::on_actionPortugueseTr_triggered()
{
    changeAppLanguage( "pt" , ui->actionPortugueseTr );
}

/** Change the app language to Chinese: */
void MainWindow::on_actionChineseTr_triggered()
{
    changeAppLanguage( "zh" , ui->actionChineseTr );
}
