#include "findroomdialog.h"
#include "ui_findroomdialog.h"

FindRoomDialog::FindRoomDialog(UserSettings *settings, QWidget *parent) :
    FindRoom(settings, parent),
    ui(new Ui::FindRoomDialog)
{
    ui->setupUi(this);

    /*
     * Suppose the max number is 10. That means if after 10 rooms
     * in a row that the user was not able to connect, the search
     * stops.
     */
    setMaxNumberOfConsecutiveFails(MAX_NUMBER_OF_CONSECUTIVE_FAILS);

    /*
     * This is the number of ips fetched in the same time from the
     * web server. More ips fetched, more it takes to arrive and
     * more bandwith used.
     */
    setTotalIpsToFetch(TOTAL_IPS_TO_FETCH);

    /*
     * The max. number of seconds waiting for the list to arrive before
     * cancelling the search due to connection problems.
     */
    setSecondsWaitingTime(TIME_WAITING_LIST_SECONDS);

    /*
     * Each connection is done in a different thread. This is the number
     * of concurrent threads trying new connections while searching for
     * rooms.
     */
    setMaxNumberOfRunningTasks(MAX_NUMBER_OF_RUNNING_TASKS);

    /* Ask the superclass to start the view: */
    setupFindRoomUI();
}

FindRoomDialog::~FindRoomDialog()
{
    delete ui;
}

void FindRoomDialog::linkViewComponents(QPushButton *&stopSearchBtn,   QPushButton *&joinRoomBtn,
                                        QPushButton *&joinIpServerBtn, QPushButton *&settingsBtn,
                                        QPushButton *&refreshListBtn,  QPushButton *&pingIpServerBtn,
                                        QLabel *&searchRoomInfoLb,     QLabel *&listTitleLb,
                                        QLabel *&nicknameLb,           QTableWidget *&listTable)
{
    stopSearchBtn = ui->stopSearchBtn;
    joinRoomBtn = ui->joinRoomListBtn;
    joinIpServerBtn = ui->joinServerBtn;
    settingsBtn = ui->settingsBtn;
    refreshListBtn = ui->refreshListBtn;
    pingIpServerBtn = ui->pingBtn;
    searchRoomInfoLb = ui->searchRoomInfoLb;
    listTitleLb = ui->listTitleLb;
    nicknameLb = ui->nicknameLb;
    listTable = ui->roomsTableWidget;
}

void FindRoomDialog::on_pingBtn_clicked() // "Ping" button clicked on connection by IP.
{
    pingIpServer(ui->ipLineEdit->text(), ui->portSpinBox->value());
}

void FindRoomDialog::on_joinServerBtn_clicked() //"Join Server" button clicked on connection by IP.
{
    connectToIpServer(ui->ipLineEdit->text(), ui->portSpinBox->value());
}

void FindRoomDialog::on_refreshListBtn_clicked()
{
    refreshRoomList();
}

void FindRoomDialog::on_joinRoomListBtn_clicked()
{
    joinSelectedRoom();
}

void FindRoomDialog::on_settingsBtn_clicked()
{
    openUserSettingsDialog();
}

void FindRoomDialog::on_stopSearchBtn_clicked()
{
    stopSearch();
}

/** Retranslates the app if its language changed: */
void FindRoomDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        setupRoomListHeaderLabels(); //refresh the title of the columns
    }
    QDialog::changeEvent(event);
}
