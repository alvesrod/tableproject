#include "tableitem.h"

TableItem::TableItem(QString name, QWidget *parent) :
    QWidget(parent)
{
    this->name = name;

    /* Declaring initial variables: */
    id = 0;
    type = -1;
    owner = NULL;
    ownerID = -1;
    rotation = 0;
    tableRotation = 0;
    currentZPosition = 0;
    youAreTheOwner = false;
    showingLockFeedback = false;
    locked = false;
    hidden = false;

    /* Controls the item moves: */
    moveFlow = new MoveFlow(pos(), this);
    connect(moveFlow, SIGNAL(move(QPoint)), this, SLOT(moveReceived(QPoint)));

    /* Make the background transparent: */
    setAttribute(Qt::WA_TranslucentBackground);

    /* Variables from the context menu: */
    hideUnhideAction = NULL;
    lockUnlockAction = NULL;
    setOwnerAction = NULL;
    removeOwnerAction = NULL;
    shuffleAction = NULL;
    refreshAction = NULL;
    removeAction = NULL;
    rotateAction = NULL;
}

/** The item will start moving to the new position (gradually): */
void TableItem::step(QPoint position)
{
    moveFlow->moveTo(position);
}

/** The item will instantly move to the new position: */
void TableItem::move(QPoint position)
{
    moveFlow->instantMove(position);
}

/** Called by the MoveFlow that is controlling this class' moves: */
void TableItem::moveReceived(QPoint position)
{
    QWidget::move(position);
    emit moveFinalized(); //warn the table, so it might update its boundaries.
}


QString TableItem::makeFilepath(QString name, QString dir, QString fileExt, int extension)
{
    QString path = dir + name;
    QString type = "." + fileExt;
    if (extension < 2) return path + type;
    else return path + " (" + QString::number(extension) + ")" + type;
}

int TableItem::getAvailableFileExtension(QString fileExt)
{
    QString path;
    int repetitions = 0;
    do {
        repetitions++;
        path = makeFilepath(name, getDirectory(), fileExt, repetitions);
    }
    while (QFile(path).exists());
    return repetitions;
}

/** Saves a file to the table item based on the @param fileData: */
QString TableItem::saveFile(QByteArray fileData, QString fileType)
{
    if (getDirectory().isEmpty()) return "";

    /* Create all the folders necessary if they don't exist: */
    QDir dir;
    dir.mkpath(getDirectory());

    /* Get the path name: */
    int extension = getAvailableFileExtension(fileType);
    QString filePath = makeFilepath(getName(), getDirectory(), fileType, extension);

    /* Save the file into the folderpath location: */
    QFile itemFile(filePath);
    if(!itemFile.open(QIODevice::WriteOnly))
        return "";

    /* Write data into the file: */
    itemFile.write(fileData);
    itemFile.flush();
    itemFile.close();

    /* Set a hash for the file to identify it: */
    setFileHash( QCryptographicHash::hash(fileData,QCryptographicHash::Md5) );
    return filePath;
}

void TableItem::setPreparingStatus()
{
    onPreparingStatus();
    status = STATUS_PREPARING;
}

void TableItem::setReceivingStatus(int progress, RoomMember *user)
{
    onReceivingStatus(progress, user);
    status = STATUS_RECEIVING;
}

void TableItem::setReadyStatus()
{
    status = STATUS_READY;
    onReadyStatus();
}

void TableItem::setErrorStatus(QString error)
{
    onErrorStatus(error);
    status = STATUS_ERROR;
}


/* Dealing with sending / receiving a package containing this class: */

QByteArray TableItem::getDataPackage()
{
    QByteArray message;
    QDataStream messageStream(&message, QIODevice::WriteOnly);
    messageStream << name;
    messageStream << pos();
    messageStream << fileHash;
    messageStream << id;
    messageStream << teamsHidden;
    messageStream << teamsLocked;
    messageStream << rotation;
    messageStream << currentZPosition;
    messageStream << ownerID;
    addDataToStream(messageStream);
    return message;
}


void TableItem::setDataPackage(QByteArray message)
{
    QPoint position;
    QDataStream messageStream(message);
    messageStream >> name;
    messageStream >> position;
    messageStream >> fileHash;
    messageStream >> id;
    messageStream >> teamsHidden;
    messageStream >> teamsLocked;
    messageStream >> rotation;
    messageStream >> currentZPosition;
    messageStream >> ownerID;

    move(position);
    useDataFromStream(messageStream);
    emit dataUpdated();
}

/* Hide and Lock features of an item: */

void TableItem::setHiddenTeams(QList<qint8> teams)
{
    if (!teamsHidden.isEmpty())
        teamsHidden.clear();
    teamsHidden = teams;
}

void TableItem::setLockedTeams(QList<qint8> teams)
{
    if (!teamsLocked.isEmpty())
        teamsLocked.clear();
    teamsLocked = teams;
}

QList<qint8> TableItem::getHiddenTeams() { return teamsHidden; }
QList<qint8> TableItem::getLockedTeams() { return teamsLocked; }
bool TableItem::isHidden(qint8 team) { return teamsHidden.contains(team); }
bool TableItem::isHiddenForYou() { return hidden; }
bool TableItem::isLocked(qint8 team) { return teamsLocked.contains(team); }
bool TableItem::isLockedForYou() { return locked; }

void TableItem::changeHiddenStatus(bool hidden)
{
    this->hidden = hidden;
    onItemHideChanged(hidden);
    emit itemHideStatusChanged(this, hidden);

}

void TableItem::changeLockStatus(bool locked)
{
    this->locked = locked;
    rotationSetEnabled(!locked);
    onItemLockChanged(locked);
}

bool TableItem::checkLock()
{
    if (locked) {
        if (!showingLockFeedback) {
            showingLockFeedback = true;
            emit requestLockFeedBack(this, true);
            QTimer::singleShot(DISPLAY_LOCK_MILLISECONDS, this, SLOT(lockFeedBackExpired()));
        }
    }
    return locked;
}

void TableItem::lockFeedBackExpired()
{
    showingLockFeedback = false;
    emit requestLockFeedBack(this, false);
}

/* Enabling/Disabling the buttons of the context menu: */

void TableItem::hideUnhideSetEnabled(bool enabled)
{
    hideUnhideAction->setEnabled(enabled);
}

void TableItem::lockUnlockSetEnabled(bool enabled)
{
    lockUnlockAction->setEnabled(enabled);
}

void TableItem::removeSetEnabled(bool enabled)
{
    removeAction->setEnabled(enabled);
}

void TableItem::refreshSetEnabled(bool enabled)
{
    refreshAction->setEnabled(enabled);
}

void TableItem::refreshSetVisible(bool visible)
{
    refreshAction->setVisible(visible);
}

void TableItem::rotationSetEnabled(bool enabled)
{
    rotateAction->setEnabled(enabled);
}

void TableItem::shuffleSetEnabled(bool enabled)
{
    shuffleAction->setEnabled(enabled);
}

bool TableItem::hideUnhideIsEnabled()
{
    return hideUnhideAction->isEnabled();
}

bool TableItem::lockUnlockIsEnabled()
{
    return lockUnlockAction->isEnabled();
}

bool TableItem::removeIsEnabled()
{
    return removeAction->isEnabled();
}

/* Public Getters and setters: */
QString TableItem::getName() { return name; }
void TableItem::setDirectory(QString tableDirectory) { directory = tableDirectory; }
QString TableItem::getDirectory(){ return directory; }
QByteArray TableItem::getFileHash() { return fileHash; }
void TableItem::setFileHash(QByteArray hash){ fileHash = hash; }
qint32 TableItem::getOwnerID() { return ownerID; }
RoomMember *TableItem::getOwner() { return owner; }
quint32 TableItem::getItemId() { return id; }
qint16 TableItem::getType() { return type; }
void TableItem::setType(qint16 itemType) { type = itemType; }
TableItem::ItemStatus TableItem::getStatus() { return status; }
QMenu *TableItem::getContextMenu() { return contextMenu; }
qint16 TableItem::getRotation() { return rotation; }
void TableItem::setRotation(qint16 angle) { rotation = angle; }
qreal TableItem::getZPosition() { return currentZPosition; }
void TableItem::setZPosition(qreal zPosition) { currentZPosition = zPosition; }
QPixmap TableItem::getItemImage() { return itemImage; }
void TableItem::setItemImage(QPixmap image) { itemImage = image; }

QKeySequence TableItem::getHideShorcut() { return QKeySequence(Qt::CTRL + Qt::Key_E); }
QKeySequence TableItem::getLockShorcut() { return QKeySequence(Qt::CTRL + Qt::Key_F); }
QKeySequence TableItem::getOwnerShorcut() { return QKeySequence(Qt::CTRL + Qt::Key_G); }
QKeySequence TableItem::getRemoveShorcut() { return QKeySequence(Qt::Key_Delete); }
QKeySequence TableItem::getRemoveOwnerShorcut() { return QKeySequence(Qt::CTRL + Qt::Key_R); }

void TableItem::setOwner(RoomMember *user)
{
    if (user == NULL) cleanOwner();
    else {
        youAreTheOwner = user->isThatYou();
        ownerID = user->getUniqueID();
        owner = user;
        connect(owner, SIGNAL(nicknameChanged(User*,User*,QString)),
                this, SLOT(ownerChangedNickname(User*,User*,QString)));
        handleOwnerContextMenu();
    }
}

void TableItem::cleanOwner()
{
    ownerID = -1;
    youAreTheOwner = false;
    if (owner) {
        disconnect(owner, SIGNAL(nicknameChanged(User*,User*,QString)),
                this, SLOT(ownerChangedNickname(User*,User*,QString)));
        owner = NULL;
    }
    handleOwnerContextMenu();
}

/** Set the item id: */
void TableItem::setItemId(quint32 id, RoomMember *user)
{
    if (!user->isTheHost())
        qWarning() << "[TableItem] Only the host user can set the item id.";
    else this->id = id;
}

void TableItem::contextMenuRequest(QPoint) {
    emit requestContextMenuFromTable(this);
}

/** Display the Context menu for an item: */
void TableItem::openContextMenu(){
    contextMenu->exec(QCursor::pos());
}

QString TableItem::getFilePath(QString fileType)
{
    return makeFilepath(name, directory, fileType);
}

void TableItem::createContextMenu(QWidget *widget)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    contextMenu = new QMenu(widget);
    QList<QAction *> menuActions;

    /* Reload item: */
    refreshAction = new QAction(tr("Refresh"), this);
    refreshAction->setIcon(QIcon(":/images/reload"));
    connect(refreshAction,SIGNAL(triggered()),this,SLOT(onRefreshTriggered()));
    menuActions << refreshAction;

    /* Hide item: */
    hideUnhideAction = new QAction(tr("Hide"), this);
    hideUnhideAction->setIcon(QIcon(":/images/hide"));
    hideUnhideAction->setShortcut(getHideShorcut());
    connect(hideUnhideAction,SIGNAL(triggered()),this,SLOT(onHideTriggered()));
    menuActions << hideUnhideAction;

    /* Lock item: */
    lockUnlockAction = new QAction(tr("Lock"), this);
    lockUnlockAction->setIcon(QIcon(":/images/lock"));
    lockUnlockAction->setShortcut(getLockShorcut());
    connect(lockUnlockAction,SIGNAL(triggered()),this,SLOT(onLockTriggered()));
    menuActions << lockUnlockAction;

    /* Rotate item: */
    rotateAction = new QAction(tr("Rotate"), this);
    rotateAction->setIcon(QIcon(":/images/rotateRight"));
    connect(rotateAction,SIGNAL(triggered()),this,SLOT(onRotateTriggered()));
    menuActions << rotateAction;

    /* Become owner: */
    setOwnerAction = new QAction(tr("Possess"), this);
    setOwnerAction->setIcon(QIcon(":/images/addUser"));
    setOwnerAction->setShortcut(getOwnerShorcut());
    connect(setOwnerAction,SIGNAL(triggered()),this,SLOT(onSetOwnerTriggered()));
    menuActions << setOwnerAction;

    /* Remove ownership: */
    removeOwnerAction = new QAction(tr("Give away"), this);
    removeOwnerAction->setIcon(QIcon(":/images/removeUser"));
    removeOwnerAction->setShortcut(getRemoveOwnerShorcut());
    connect(removeOwnerAction,SIGNAL(triggered()),this,SLOT(onRemoveOwnerTriggered()));
    menuActions << removeOwnerAction;

    /* Shuffle items: */
    shuffleAction = new QAction(tr("Shuffle"), this);
    shuffleAction->setIcon(QIcon(":/images/shuffle"));
    connect(shuffleAction,SIGNAL(triggered()),this,SLOT(onShuffleTriggered()));
    menuActions << shuffleAction;
    shuffleAction->setDisabled(true);

    /* Remove item: */
    removeAction = new QAction(tr("Remove"), this);
    removeAction->setIcon(QIcon(":/images/remove"));
    removeAction->setShortcut(getRemoveShorcut());
    connect(removeAction,SIGNAL(triggered()),this,SLOT(onRemoveTriggered()));
    menuActions << removeAction;

    /* Execute the context menu in the mouse position: */
    contextMenu->addActions(menuActions);

    connect(this,SIGNAL(customContextMenuRequested(QPoint)),
            this,SLOT(contextMenuRequest(QPoint)));

    handleOwnerContextMenu();
}


/* Slots called by the actions in the context menu: */

void TableItem::onHideTriggered()
{
    if (checkLock()) return; //Can't hide locked items
    QSound::play("Sound/itemMenu.wav");
    emit requestHideDialog(this);
}

void TableItem::onLockTriggered()
{
    QSound::play("Sound/itemMenu.wav");
    emit requestLockDialog(this);
}

void TableItem::onRemoveTriggered()
{
    if (checkLock()) return; //Can't remove locked items
    QSound::play("Sound/itemRemove.wav");
    emit requestRemoval(this);
}

void TableItem::onRefreshTriggered()
{
    QSound::play("Sound/itemMenu.wav");
    load();
}

void TableItem::onRotateTriggered()
{
    if (checkLock()) return; //Can't rotate locked items
    QSound::play("Sound/itemMenu.wav");
    emit requestRotation(this, rotation + 90);
}

void TableItem::onSetOwnerTriggered()
{
    QSound::play("Sound/itemMenu.wav");
    emit requestOwnership(this, true);
}

void TableItem::onRemoveOwnerTriggered()
{
    QSound::play("Sound/itemMenu.wav");
    emit requestOwnership(this, false);
}

void TableItem::onShuffleTriggered()
{
    if (checkLock()) return; //Can't shuffle locked items
    emit requestShuffle(this);
}

void TableItem::displayOwnerToolTip()
{
    if ( (owner == NULL) || (youAreTheOwner) ) setToolTip("");
    else setToolTip(tr("Owner:") + " <b>" + owner->getNickname() + "</b>");
}


bool TableItem::hasOwner()
{
    return (ownerID > 0);
}

/** Retranslates the app if its language changed: */
void TableItem::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        displayOwnerToolTip();
    }
    QWidget::changeEvent(event);
}

void TableItem::ownerChangedNickname(User *memberChanged, User *, QString)
{
    memberChanged = dynamic_cast<RoomMember*>(memberChanged);
    if ( (memberChanged != NULL) && (memberChanged->getUniqueID() == ownerID) )
        displayOwnerToolTip();
}

void TableItem::handleOwnerContextMenu()
{
    if ( (setOwnerAction == NULL) || (removeOwnerAction == NULL) ) return;
    setOwnerAction->setVisible(!youAreTheOwner);
    removeOwnerAction->setVisible(youAreTheOwner);
    removeOwnerAction->setEnabled(youAreTheOwner);
    if (youAreTheOwner)
        setOwnerAction->setEnabled(false);
    else
        setOwnerAction->setEnabled(!hasOwner());
    displayOwnerToolTip();
}
