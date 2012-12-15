#include "findroom.h"

FindRoom::FindRoom(UserSettings *settings, QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    if (settings == NULL) {
        qWarning() << "Cannot initialize FindRoom without User Settings.";
        return;
    }

    /* Connect to the WebContact to receive the list of ips from the web server: */
    connect(WebContact::getInstance(), SIGNAL(roomListRequested(QStringList)), this, SLOT(ipListArrived(QStringList)), Qt::QueuedConnection);

    /* Connect to the WebContact to request the list of ips: */
    connect(this, SIGNAL(requestRooms(qint32)), WebContact::getInstance(), SLOT(requestPublishedRooms(qint32)), Qt::QueuedConnection);

    /* Make sure you update the nickname if the user changes his name on the settings dialog: */
    connect(settings, SIGNAL(nicknameChanged(QString)), this, SLOT(nicknameChanged(QString)));

    /* Declare private variables: */
    maxConsecutiveFails = DEFAULT_MAX_NUM_OF_CONSECUTIVE_FAILS;
    totalIpsToFecth = DEFAULT_TOTAL_IPS_TO_FETCH;
    secondsWaitingTime = DEFAULT_TIME_WAITING_LIST_SECONDS;
    maxRunningTasks = DEFAULT_MAX_NUM_RUNNING_TASKS;
    roomsFound = 0;
    roomsPinged = 0;
    consecutivePingFails = 0;
    serverResponded = false;
    searchedRoomIsPasswordProtected = false;
    userSettings = settings;

    connect(&searchTimer, SIGNAL(timeout()), this, SLOT(timeExpired()));

    declareNullComponents();
}

UserSettings *FindRoom::getUserSettings()
{
    return userSettings;
}

void FindRoom::openUserSettingsDialog()
{
    userSettings->openDialog();
}

/**
    Try to join the room that is
    currently selected in the room list table:
  */
void FindRoom::joinSelectedRoom()
{
    roomSelected(listTable->currentRow(), listTable->currentColumn());
}

void FindRoom::refreshUserNicknameLabel()
{
    //qDebug() << "Setting nickname to " << userSettings->getNickname();
    nicknameLb->setText( userSettings->getNickname() );
}

void FindRoom::pingIpServer(QString ip, int port)
{
    pingIpServerBtn->setEnabled(false);
    searchRoomInfoLb->setText(tr("Searching..."));
    PingRoomTask *task = new PingRoomTask(ip, port, this);
    connect(task, SIGNAL(result(RoomDescription*,PingRoomTask*)), this,
                  SLOT(directConnectionPingResult(RoomDescription*,PingRoomTask*)), Qt::QueuedConnection);
    task->startTask();
    runningTasks << task;
}

void FindRoom::connectToIpServer(QString ip, int port)
{
    connectToRoom(ip, port, searchedRoomIsPasswordProtected);
}

void FindRoom::ping(QString ip, int port)
{
    qDebug() << "Pinging... " << ip << ":" << port;
    PingRoomTask *task = new PingRoomTask(ip, port, this);
    connect(task, SIGNAL(result(RoomDescription*,PingRoomTask*)), this,
                  SLOT(oneListConnectionPingResult(RoomDescription*,PingRoomTask*)), Qt::QueuedConnection);
    task->startTask();
    runningTasks << task;
}

/** Select a room from the table and try to join it: */
void FindRoom::roomSelected(int row, int column)
{
    if (row < 0) return;

    QString ip = listTable->item(row, COLUMN_IP)->text();
    qint32 port = (listTable->item(row, COLUMN_PORT)->text()).toInt();
    bool hasPassword = (listTable->item(row, COLUMN_PASSWORD) != NULL);
    qDebug() << "Room selected:" << " " << row << "," << column << ") "
             << "Connecting to" << " " << ip << ":" << port;
    connectToRoom(ip, port, hasPassword);
}

/** Try to join a room: */
void FindRoom::connectToRoom(QString ip, int port, bool passwordProtected)
{
    RoomDescription *roomDescription = Engine::newRoomDescription(port,ip);
    RoomController *room = new RoomController( roomDescription , userSettings );

    /* Check if room is password protected. If yes, type the password: */
    if (passwordProtected)
        roomDescription->setPassword( getPassword() );

    /* Close this window and signal to join the room: */
    this->close();
    emit joinRoom(room);
}

void FindRoom::setListTitle(int rooms, bool searching)
{
    QString title = QString::number(rooms) + " " + tr("room") + ((rooms!=1)?tr("s"):tr("")) + " " + tr("found.") + " ";
    if (searching) title.append(tr("Searching..."));
    listTitleLb->setText(title);
}

void FindRoom::nicknameChanged(QString name)
{
    nicknameLb->setText(name);
}

void FindRoom::timeExpired()
{
    if (serverResponded) return;
    searchTimer.stop();
    listTitleLb->setText(tr("The server did not respond."));
    refreshListBtn->setEnabled(true);
    stopSearchBtn->setEnabled(false);
}

void FindRoom::setMaxNumberOfConsecutiveFails(int max)
{
    maxConsecutiveFails = max;
}

void FindRoom::setMaxNumberOfRunningTasks(int max)
{
    maxRunningTasks = max;
}

void FindRoom::setSecondsWaitingTime(int seconds)
{
    secondsWaitingTime = seconds;
}

void FindRoom::setTotalIpsToFetch(int total)
{
    totalIpsToFecth = total;
}

void FindRoom::setupFindRoomUI()
{
    linkViewComponents(stopSearchBtn,    joinRoomBtn,    joinIpServerBtn,
                       settingsBtn,      refreshListBtn, pingIpServerBtn,
                       searchRoomInfoLb, listTitleLb,    nicknameLb,
                       listTable);
    refreshUserNicknameLabel();
    createRoomsList();
    refreshRoomList();
}

/**
    Called when the class receives
    from the webserver a list of ips
    from the rooms. It grabs the list
    and start pinging them to make sure
    they are opened and get more info:
*/
void FindRoom::ipListArrived(QStringList list)
{
    searchTimer.stop();
    serverResponded = true;
    roomsFound = list.size();
    roomsPinged = 0;
    waitingRoomsToPing = list;
    if (list.isEmpty()) {
        setListTitle(0);
        stopSearch();
        return;
    }

    setListTitle(0, true);
    pingSomeRooms();
}

/**
  The purpose of this function is to set a small part of the list of ips
  to starting pinging them. If there
  are too much fails, it will stop pinging.
  */
void FindRoom::pingSomeRooms()
{
    if ( (waitingRoomsToPing.isEmpty()) ||
         (consecutivePingFails >= maxConsecutiveFails) ) {
        stopSearch();
        return;
    }

    while (runningTasks.size() < maxRunningTasks) {
        QString addr = waitingRoomsToPing.at(0);
        QStringList host = addr.split(" "); //Because ip and port are separated by space
        ping(host.at(0), host.at(1).toInt());
        waitingRoomsToPing.removeFirst();
        if (waitingRoomsToPing.isEmpty()) break;
    }
}

void FindRoom::stopSearch()
{
    stopSearchBtn->setEnabled(false);
    refreshListBtn->setEnabled(true);
    waitingRoomsToPing.clear();
    consecutivePingFails = 0;
    setListTitle(listTable->rowCount());
}

void FindRoom::directConnectionPingResult(RoomDescription *description, PingRoomTask *task)
{
    if (runningTasks.indexOf( task ) < 0) return;
    pingIpServerBtn->setEnabled(true);
    if (description == NULL)
        searchRoomInfoLb->setText( "<b>" + tr("Status:") + " <font color='#883333'>" + tr("Offline") + "</font></b>" );
    else {
        searchedRoomIsPasswordProtected = description->hasPassword();
        joinIpServerBtn->setEnabled(true);
        searchRoomInfoLb->setText( QString("<b>" + tr("Status:") + " <font color='#338833'>" + tr("Online") +
                                           "</font></b> " + tr("since") + " ")
                                       + description->getCreationTime().toString()
                                       + "<br><b>" + tr("Host user:") + "</b> " + description->getHostNicknamePrediction()
                                       + "<br><b>" + tr("Room:") + "</b> " + description->getRoomName()
                                       + "<br><b>" + tr("Type:") + "</b> " + description->getRoomType()
                                       + "<br><b>" + tr("Users online:") + "</b> "
                                       + QByteArray::number(description->getTotalUsersInRoom())
                                       + "/" + QByteArray::number(description->getMaxNumberOfUsers())
                                       + "<br><b>" + tr("Location:") + "</b> " + description->getLocation()
                                       + "<br><b>" + tr("Password Protected:") + "</b> "
                                       + ((searchedRoomIsPasswordProtected)?tr("Yes"):tr("No"))
                                       + "<br><b>" + tr("Latency:") + "</b> " + QString::number(description->getRoomLatency())
                                       + " ms");
        description->deleteLater();
    }
    runningTasks.removeOne(task);
}

void FindRoom::oneListConnectionPingResult(RoomDescription *description, PingRoomTask *task)
{
    if (runningTasks.indexOf( task ) < 0) return;
    roomsPinged++;
    bool searching = (roomsPinged < roomsFound);
    if (description == NULL) {
        setListTitle(listTable->rowCount(), searching);
        consecutivePingFails++;
    } else {
        consecutivePingFails = 0;
        QString listDetail = getListDetail(description);
        if (!listDetails.contains(listDetail)) {
            listDetails << listDetail;
            addTableRow(description);
            setListTitle(listTable->rowCount(), searching);
        }
        description->deleteLater();
    }
    runningTasks.removeOne(task);
    pingSomeRooms();
}

/** Adds a row to the table based on the info about the room: */
void FindRoom::addTableRow(RoomDescription *description)
{
    listTable->insertRow(0);
    listTable->setItem(0,COLUMN_ROOM,new QTableWidgetItem(description->getRoomName()));
    listTable->setItem(0,COLUMN_TYPE,new QTableWidgetItem(description->getRoomType()));
    listTable->setItem(0,COLUMN_HOST,new QTableWidgetItem(description->getHostNicknamePrediction()));
    listTable->setItem(0,COLUMN_USERS,new QTableWidgetItem(QString::number(description->getTotalUsersInRoom())
                                      + "/" + QString::number(description->getMaxNumberOfUsers())));
    listTable->setItem(0,COLUMN_LATENCY,new QTableWidgetItem(QString::number(description->getRoomLatency())));
    listTable->setItem(0,COLUMN_LOCATION,new QTableWidgetItem(description->getLocation()));
    listTable->setItem(0,COLUMN_HOSTED_SINCE, new QTableWidgetItem(description->getCreationTime().toString()));
    listTable->setItem(0,COLUMN_IP, new QTableWidgetItem(description->getRoomIp()));
    listTable->setItem(0,COLUMN_PORT, new QTableWidgetItem(QString::number(description->getRoomPort())));

    if (description->hasPassword()) {
        listTable->setItem(0,COLUMN_PASSWORD, new QTableWidgetItem(QIcon(":/images/protected"), ""));
        listTable->item(0, COLUMN_PASSWORD)->setToolTip(tr("Password protected"));
    }
}

/** Return a string with all the room information to be able to compare with other rooms: */
QString FindRoom::getListDetail(RoomDescription *description)
{
    return QString(description->getRoomName() + description->getRoomType()
                   + description->getHostNicknamePrediction() + description->getMaxNumberOfUsers()
                   + description->getLocation() + description->getRoomPort()
                   + description->getCreationTime().toString());
}

/** Sets the size of each column from the room list: */
void FindRoom::setUpColumnSizes()
{
    listTable->setColumnWidth(COLUMN_PASSWORD, COLUMN_PASSWORD_SIZE);
    listTable->setColumnWidth(COLUMN_USERS, COLUMN_USERS_SIZE);
    listTable->setColumnWidth(COLUMN_HOST, COLUMN_HOST_SIZE);
    listTable->setColumnWidth(COLUMN_ROOM, COLUMN_ROOM_SIZE);
    listTable->setColumnWidth(COLUMN_TYPE, COLUMN_TYPE_SIZE);
    listTable->setColumnWidth(COLUMN_LOCATION, COLUMN_LOCATION_SIZE);
    listTable->setColumnWidth(COLUMN_LATENCY, COLUMN_LATENCY_SIZE);
    listTable->setColumnWidth(COLUMN_HOSTED_SINCE, COLUMN_HOSTED_SINCE_SIZE);
}

void FindRoom::setupRoomListHeaderLabels()
{
    listTable->setHorizontalHeaderItem(COLUMN_PASSWORD, new QTableWidgetItem(QIcon(":/images/key"), QString("")));
    listTable->setHorizontalHeaderItem(COLUMN_USERS, new QTableWidgetItem(QIcon(":/images/users"),QString("")));
    listTable->setHorizontalHeaderItem(COLUMN_ROOM, new QTableWidgetItem(QString(tr("Room"))));
    listTable->setHorizontalHeaderItem(COLUMN_TYPE, new QTableWidgetItem(QString(tr("Type"))));
    listTable->setHorizontalHeaderItem(COLUMN_HOST, new QTableWidgetItem(QString(tr("Host"))));
    listTable->setHorizontalHeaderItem(COLUMN_LATENCY, new QTableWidgetItem(QString(tr("Latency"))));
    listTable->setHorizontalHeaderItem(COLUMN_LOCATION, new QTableWidgetItem(QString(tr("Location"))));
    listTable->setHorizontalHeaderItem(COLUMN_HOSTED_SINCE, new QTableWidgetItem(QString(tr("Hosted since"))));

    /* Set up tooltips: */
    listTable->horizontalHeaderItem(COLUMN_USERS)->setToolTip(tr("Total users in the room"));
    listTable->horizontalHeaderItem(COLUMN_PASSWORD)->setToolTip(tr("Is the room password protected?"));
}

void FindRoom::createRoomsList()
{
    listTable->setColumnCount(TOTAL_COLUMNS);
    listTable->setColumnHidden(COLUMN_IP, true); //Hide ip
    listTable->setColumnHidden(COLUMN_PORT, true); //Hide port

    /* Create header labels: */
    setupRoomListHeaderLabels();

    connect ( listTable, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(roomSelected(int,int)));
    listTable->sortByColumn(COLUMN_LATENCY, Qt::DescendingOrder);

    /* Set the size of the columns: */
    setUpColumnSizes();
}

void FindRoom::declareNullComponents()
{
    stopSearchBtn = NULL;
    joinRoomBtn = NULL;
    joinIpServerBtn = NULL;
    settingsBtn = NULL;
    refreshListBtn = NULL;
    pingIpServerBtn = NULL;
    searchRoomInfoLb = NULL;
    listTitleLb = NULL;
    nicknameLb = NULL;
    listTable = NULL;
}

/**
  Opens an input dialog for the user to type the room password.
  @return the password typed.
 */
QString FindRoom::getPassword()
{
    bool ok;
    QString password = QInputDialog::getText(this, tr("Room has password"),tr("Room password:"), QLineEdit::Password,"", &ok);
    if (!ok || password.isEmpty()) return "";
    else return password;
}

void FindRoom::removeAllRunningTasks()
{
    foreach(PingRoomTask *task, runningTasks) {
        task->exit();
    }
    runningTasks.clear();
}

void FindRoom::refreshRoomList()
{
    refreshListBtn->setEnabled(false);
    listTitleLb->setText(tr("Searching for rooms..."));
    removeAllRunningTasks();
    listTable->clearContents();
    listDetails.clear();
    listTable->setRowCount(0);

    emit requestRooms(totalIpsToFecth);

    searchTimer.stop();
    searchTimer.start(secondsWaitingTime*1000);
    serverResponded = false;
    stopSearchBtn->setEnabled(true);
}
