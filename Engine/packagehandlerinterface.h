#ifndef PACKAGEHANDLERINTERFACE_H
#define PACKAGEHANDLERINTERFACE_H

#include <QByteArray>
#include <QDataStream>

/**
 * @brief This is an inferface for all the classes in the Engine
 * than send information through the network. This cannot be
 * accessed from outside the Engine.
 */

class IPackageHandler
{

public:
    /**
     * @return a QByteArray with all the information the class wants to send
     * through the network:
     */
    virtual QByteArray getDataPackage() = 0;

    /**
     * @param message is the QByteArray received (from other machine's getDataPackage())
     * So, open this message and handle it:
     */
    virtual void setDataPackage(QByteArray message) = 0;

signals:
    /**
     * This signal should be send after setDataPackage() sets the data received:
     */
    virtual void dataUpdated() = 0;

};

/**
 * Overloading the << and >> operators, so the class can easily be added into a QDataStream:
 */
inline QDataStream & operator<<(QDataStream & ds, IPackageHandler* packageHandler) {
    if (packageHandler == NULL) ds << QByteArray("");
    else ds << packageHandler->getDataPackage();
    return ds;
}

inline QDataStream & operator>>(QDataStream & ds, IPackageHandler* packageHandler) {
    QByteArray message;
    ds >> message;
    if (packageHandler != NULL)
        packageHandler->setDataPackage(message);
    return ds;
}

#endif // PACKAGEHANDLERINTERFACE_H
