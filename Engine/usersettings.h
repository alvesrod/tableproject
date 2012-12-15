#ifndef USERSETTINGS_H
#define USERSETTINGS_H

#include <QObject>
#include <QColor>
#include <QDebug>
#include <QFont>
#include <QSettings>
#include <QNetworkInterface>
#include <QFlags>

#include "Engine_global.h" /* This class can be used outside the Engine */
#include "packagehandlerinterface.h" /* Inteface to allow this class to handle network packages */
#include "webcontact.h"

#define USER_SETTINGS_FOLDER "UserSettings"
#define DEFAULT_NICKNAME "User"
#define DEFAULT_UNDEFINED_NICKNAME "[Unknown]"
#define DEFAULT_COLOR "#000000" //Black
#define DEFAULT_FONT "MS Shell Dlg 2"
#define DEFAULT_FONT_SIZE 12

#define SEND_SIGNAL_AND_SAVE true
#define OMIT_SIGNAL_DONT_SAVE false

/**
 * @brief
 * UserSettings is the data from the user computer, such as the user nickname
 * stored in the register file, or the type of font being used in the chat.
 * This class is extended outside the Engine to implement more specific settings
 * about a particular user stored in the user computer.
 */

class ENGINESHARED_EXPORT UserSettings : public QObject, public IPackageHandler
{
    Q_OBJECT
public:
    UserSettings(QObject *parent = 0, bool isAnotherUser = false);

    /* Getters and setters for user settings variables: */
    //done
    void setNickname(QString name, bool signal = SEND_SIGNAL_AND_SAVE);
    //done
    QString getNickname();
    //done
    QString getDefaultNickname();


    //done
    void setChatColor(QColor color, bool signal = SEND_SIGNAL_AND_SAVE);
    //done
    QColor getChatColor();
    //done
    QColor getDefaultChatColor();


    //done
    void setChatFont(QFont font, bool signal = SEND_SIGNAL_AND_SAVE);
    //done
    QFont getChatFont();
    //done
    QFont getDefaultFont();


    void openDialog();


    QString getIp();
    QString getLocation();

    /*
     * Methods from the PackageHandler interface that this class implements.
     * Those methods deal with all the shared class variables as a single ByteArray
     * package (necessary to send through the network). The operators << and >>
     * are overloaded to call those 2 functions. One prepares the package based on the
     * values of the variables. The other reads the package in the parameter and assign
     * values to the shared variables based on that package:
     */
    QByteArray getDataPackage();
    void setDataPackage(QByteArray message);

    /*
     * Get a local list of all ips good enough for online interactions.
     * This doesn't depend on the webserver. It's good as a temporary
     * data until the "right" ip arrives from the webserver. I mean, the
     * one that is actually being used to contact the internet. This
     * function might return an empty list if there are no internet ips.
     */
    static QStringList getMyIps();

signals:

    /* Those Changed signals are emitted when the user changed them through the settings dialog: */
    void nicknameChanged(QString nickname);
    void chatColorChanged(QColor color);
    void chatFontChanged(QFont font);

    /** Signal emitted asking the settings dialog to be opened: */
    void openSettingsDialog();

    /** Signal emmited when this class was updated based on a network package: */
    void dataUpdated();

protected:
    /*
     * To be implemented by the class that inherits this one.
     * addDataToStream() is just inside getDataPackage() to add
     * more data to the package (such as new variables not implemented
     * in this class). useDataFromStream() is inside setDataPackage() and
     * behaves like it. The difference is that those 2 functions are implemented
     * by the class that inherits this one while getDataPackage() and setDataPackage()
     * are only implemented by this class.
     */
    virtual void addDataToStream(QDataStream &stream) = 0;
    virtual void useDataFromStream(QDataStream &stream) = 0;
    
private slots:
    /*
     * WebContact can emit a signal that changes the ip and location.
     * The user settings might capture this signal because the user
     * ip and location are part of the user settings:
     */
    void setIp(QString newIp);
    void setLocation(QString newLocation);

    /* No worries about private stuff: */

private:
    void loadLocalSettings();
    void loadDefaultSettings();
    QString getAddress(QString variable);

    void loadChatColor();
    void loadChatFont();
    void loadNickname();
    void saveChatColor();
    void saveChatFont();
    void saveNickname();

    QColor chatColor;
    QFont chatFont;
    QString nickname;
    QString ip;
    QString location;
    bool isYou;

};

#endif // USERSETTINGS_H
