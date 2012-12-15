#include "membersettings.h"

MemberSettings::MemberSettings(QObject *parent, bool isAnotherUser) :
    UserSettings(parent, isAnotherUser)
{
    //Do things like: value = 0;
}

void MemberSettings::addDataToStream(QDataStream &) //Parameter not yet being used
{
    //Do things like: parameter << value;
}

void MemberSettings::useDataFromStream(QDataStream &) //Parameter not yet being used
{
    //Do things like: parameter >> value;
}
