#ifndef LOCALNETWORK_H
#define LOCALNETWORK_H

#include <QObject>
#include <QThread>
#include <QMessageBox>
#include <QDataStream>
#include <QTimer>

#include "tcpserver.h"

/**
 * @brief This class has a client and a server and deals with the network communications.
 * It forces only one type of class to contact it (name stored in the important constants).
 */

#define UDP_MESSAGE false
#define TCP_MESSAGE true

#define CONNECTION_TIMELIMIT 6000 //The timelimit in miliseconds to connect to the server.
#define MAX_TOTAL_SIZE 100000000 //Maximum size of a messsage that will be fragmented.
#define CLASS_THAT_CAN_CONTACT_NETWORK "RoomController" //Just one class contacts the network.
#define TOTAL_PACKAGES_SENT_AFTER_DELAY 100000000 //How many packages to send in the delay below
#define MILLISECONDS_PACKAGE_DELAY 1 //Time delay in miliseconds between writes. Increase this to simulate slow internet.


class Network : public QThread
{
    Q_OBJECT
public:
    Network(QObject *parent); //Parameter is important to check if the caller is from an expected type.
    ~Network();

protected:
    void run(); //Necessary since this class is a thread.

signals:
    void networkMessage(qint32 sender, QByteArray message); //Sent when you receive a message.
    void disconnectedFromServer(bool sucess = true); //Sent when you are disconnected.
    void errorMessage(QString error); //Sent if a crucial error happened.
    void someoneConnected(qint32 descriptor); //Sent when someoene connected to the server.
    void someoneDisconnected(qint32 descriptor); //Sent when someoene disconnected from the server.
    void serverStarted(bool serverStarted, quint16 port); //Sent when you are the server and the server started.
    void connectedToServer(bool connectedToServer); //Sent when you connected to the server.
    void messageSent(bool messageSent); //Sent when your message was sucessfully sent.
    void serverClosed(bool serverClosed = true); //Sent when the server was closed.

public slots:
    void startNewServer(qint32 port); //Called to host a new server at the @param port
    void connectToServer(QString ip, qint32 port); //Called to join a server (@param server ip and port)
    void sendMessageToServer(QByteArray message, qint32 to = BROADCAST_DESCRIPTOR, bool TCP = TCP_MESSAGE);
    void isConnectedToServer(); //Emits a signal saying whether or not the user is connected to a server.
    void closeServer(); //Closes the server and drops all the users in it.
    void disconnectFromServer(); //Disconnect yourself from a server.


/** ===== PRIVATE STUFF: ===== **/
private slots:
    void networkMessageReceived(TCPClient *to, qint32 from, QByteArray message);
    void errorWarning(QString error);
    void userDisconnected(TCPClient* clientDisconnected, bool wasServerClosed);
    void someoneDisconnectedFromHost(qint32 descriptor);
    void someoneConnectedToHost(qint32 descriptor);
    void clientConnected(bool success);
    void periodicTime();
    
private:
    void refreshServer();
    void refreshClient();
    void writeMessageInSocket(bool TCP, QByteArray newMessage, bool lastFragment);

    TCPServer *server; //Stores the server (NULL if you are not the server).
    TCPClient *client; //Stores you as the client in a room.
    QHash<qint32, QByteArray> fragments; //fragments of messages (when a message was divided into pieces).

    /* Timer queue variables: */
    QList<QByteArray> packageQueue;
    QList<bool> lastFragmentQueue;
    QList<bool> tcpQueue;
    QTimer periodicMessagesTimer;

};

#endif // LOCALNETWORK_H
