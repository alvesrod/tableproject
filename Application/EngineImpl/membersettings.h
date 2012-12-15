#ifndef MEMBERSETTINGS_H
#define MEMBERSETTINGS_H

#include "../../Engine/usersettings.h"

/**
 * This class inherits UserSettings and stores
 * specific information about the User settings
 * that is saved in the user computer.
 */

class MemberSettings : public UserSettings
{
    Q_OBJECT
public:
    MemberSettings(QObject *parent = 0, bool isAnotherUser = false);
    
signals:
    
public slots:

private:
    /**
     * Whatever you add to addDataToStream() will be sent through
     * the network to all other users in the room. You get the data
     * from all the users at the loading screen. You can grab that data
     * by using the useDataFromStream() function. Check other classes
     * where a similar implementation was used.
     */
    void addDataToStream(QDataStream &);
    void useDataFromStream(QDataStream &);
    
};

#endif // MEMBERSETTINGS_H
