#include "usersettings.h"

UserSettings::UserSettings(QObject *parent, bool isAnotherUser) :
    QObject(parent)
{

    /* Make sure the Engine is running: */
    if (!Engine::isRunning()) {
        qCritical() << "[ERROR] Engine class called, but Engine was not started!";
        return;
    }

    isYou = !isAnotherUser;
    location = DEFAULT_LOCATION;

    /* Get a random number to be the ip until the real ip arrives.
     * This is important because users might also be identified sometimes
     * through their ip numbers. It is not a reliable method of identification,
     * but can still be used in some cases that reliability isn't much of a problem
     * (such as kicking users out of a room temporarily).
     */
    setIp( QString::number(qrand()) );



    if (isYou) {

        loadLocalSettings();

        /* Get one ip from the current ips until the user has a definite ip from the WebContact: */
        QStringList tempIps = getMyIps();
        if (!tempIps.isEmpty())
            setIp( tempIps.at(0) );

        /* Get ready to receive a signal in case a new location arrives: */
        connect(WebContact::getInstance(), SIGNAL(locationReceived(QString)), this, SLOT(setLocation(QString)), Qt::QueuedConnection);

        /* Get ready to receive a signal in case a new ip arrives: */
        connect(WebContact::getInstance(), SIGNAL(ipReceived(QString)), this, SLOT(setIp(QString)), Qt::QueuedConnection);
    } else
        loadDefaultSettings();
}

void UserSettings::loadLocalSettings()
{
    loadChatColor();
    loadChatFont();
    loadNickname();
}

void UserSettings::loadDefaultSettings()
{
    chatColor = DEFAULT_COLOR;
    chatFont = QFont(DEFAULT_FONT);
    chatFont.setPointSize(DEFAULT_FONT_SIZE);
    nickname = DEFAULT_UNDEFINED_NICKNAME;
}

QString UserSettings::getAddress(QString variableName)
{
    return QString(USER_SETTINGS_FOLDER) + "/" + variableName;
}

void UserSettings::loadChatColor()
{
    if (!isYou) return;
    QSettings settings;
    if (settings.contains(getAddress("chatColor")))
        chatColor = settings.value( getAddress("chatColor") ).value<QColor>();
    else
        chatColor = DEFAULT_COLOR;
}

void UserSettings::loadChatFont()
{
    if (!isYou) return;
    QSettings settings;
    if (settings.contains(getAddress("chatFont"))) {
        chatFont = settings.value( getAddress("chatFont") ).value<QFont>();
    }
    else {
        chatFont = QFont(DEFAULT_FONT);
        chatFont.setPointSize(DEFAULT_FONT_SIZE);
    }
}

void UserSettings::loadNickname()
{
    if (!isYou) return;
    QSettings settings;
    if (settings.contains(getAddress("nickname")))
        nickname = settings.value( getAddress("nickname") ).value<QString>();
    else
        nickname = DEFAULT_NICKNAME;
}

void UserSettings::saveChatColor()
{
    if (!isYou) return;
    QSettings settings;
    settings.setValue(getAddress("chatColor"), chatColor);
}

void UserSettings::saveChatFont()
{
    if (!isYou) return;
    QSettings settings;
    settings.setValue(getAddress("chatFont"), chatFont);
}

void UserSettings::saveNickname()
{
    if (!isYou) return;
    QSettings settings;
    settings.setValue(getAddress("nickname"), nickname);
}

void UserSettings::setNickname(QString name, bool signal)
{
    nickname = name;
    if (signal == SEND_SIGNAL_AND_SAVE) {
        saveNickname();
        emit nicknameChanged(nickname);
    }
}

QString UserSettings::getNickname()
{
    return nickname;
}

QString UserSettings::getDefaultNickname()
{
    return DEFAULT_NICKNAME;
}

void UserSettings::setChatColor(QColor color, bool signal)
{
    chatColor = color;
    if (signal == SEND_SIGNAL_AND_SAVE) {
        saveChatColor();
        emit chatColorChanged(chatColor);
    }
}

QColor UserSettings::getChatColor()
{
    return chatColor;
}

QColor UserSettings::getDefaultChatColor()
{
    return QColor(DEFAULT_COLOR);
}

void UserSettings::setChatFont(QFont font, bool signal)
{
    chatFont = font;
    if (signal == SEND_SIGNAL_AND_SAVE) {
        saveChatFont();
        emit chatFontChanged(chatFont);
    }
}

QFont UserSettings::getChatFont()
{
    return chatFont;
}

QFont UserSettings::getDefaultFont()
{
    QFont font = QFont(DEFAULT_FONT);
    font.setPointSize(DEFAULT_FONT_SIZE);
    return font;
}

void UserSettings::openDialog()
{
    emit openSettingsDialog();
}

void UserSettings::setIp(QString newIp)
{
    ip = newIp;
}

QString UserSettings::getIp()
{
    return ip;
}

void UserSettings::setLocation(QString newLocation)
{
    location = newLocation;
}

QString UserSettings::getLocation()
{
    return location;
}

QByteArray UserSettings::getDataPackage()
{
    QByteArray message;
    QDataStream messageStream(&message, QIODevice::WriteOnly);
    messageStream << nickname;
    messageStream << chatColor;
    messageStream << ip;
    addDataToStream(messageStream);
    return message;
}

void UserSettings::setDataPackage(QByteArray message)
{
    QDataStream messageStream(message);
    messageStream >> nickname;
    messageStream >> chatColor;
    messageStream >> ip;
    useDataFromStream(messageStream);
    emit dataUpdated();
}

/**
  * Get the active ips of the user.
  */
QStringList UserSettings::getMyIps()
{
    QStringList ips;
    QList<QNetworkInterface> hosts = QNetworkInterface::allInterfaces();

    foreach (QNetworkInterface host, hosts) {
        QFlags<QNetworkInterface::InterfaceFlag> flag = host.flags();
        if ( (flag.testFlag(QNetworkInterface::IsUp)) &&
             (flag.testFlag(QNetworkInterface::IsRunning)) &&
             (flag.testFlag(QNetworkInterface::CanBroadcast)) &&
             (!flag.testFlag(QNetworkInterface::IsLoopBack)) &&
             (flag.testFlag(QNetworkInterface::CanMulticast)) &&
             host.isValid()) {

            QList<QNetworkAddressEntry> addresses = host.addressEntries();
            foreach(QNetworkAddressEntry address, addresses) {
                QHostAddress ip = address.ip();
                if (ip.protocol() == QAbstractSocket::IPv4Protocol)
                    ips << address.ip().toString();
            }
        }
    }
    return ips;
}



