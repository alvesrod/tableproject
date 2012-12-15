#ifndef LOCALSERVER_H
#define LOCALSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QList>
#include "tcpclient.h"

#define INVALID_HOST_DESCRIPTOR -1

/**
  @brief This represents the server that allows others to join it.
  Only the Network class that should interact with this class.
 */

class TCPServer : public QTcpServer
{
    Q_OBJECT
public:
    TCPServer(QObject *parent);
    ~TCPServer();
    bool startServer(qint32 port = 0);
    bool closeServer();

protected:
    //Function called when a client tries to connect to the server:
    void incomingConnection(int handle);
    
signals:
    void errorWarning(QString errorMessage);
    void newClient(qint32 descriptor); //new client just arrived
    
private slots:
    void messageReceivedUDP();
    void messageReceived(TCPClient *from, qint32 to, QByteArray message);
    void clientDisconnected(TCPClient *client);
private:
    void sendMessage(qint32 from, qint32 to, QByteArray message);
    void broadcastMessage(QByteArray msg);
    void sendMessageThatSomeoneConnected(qint32 descriptor);
    TCPClient* searchClient(qint32 descriptor);

    QHash<qint32, TCPClient*> clientList;
    int hostDescriptor;
    QUdpSocket *hostUDP;

};

#endif // LOCALSERVER_H
