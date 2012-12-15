#ifndef LOCALCLIENT_H
#define LOCALCLIENT_H

#include <QObject>
#include <QDebug>
#include <QTcpSocket>
#include <QUdpSocket>

/*
 * Descriptors are used to identify a socket connected to the server.
 * Since there are no negative descriptors, we use negative descriptors
 * for special messages sent from the server:
 */
#define DESCRIPTOR_CODE_FOR_HOST -2 //When this is the descriptor, it means the message is for/from the host.
#define DESCRIPTOR_CODE_FOR_NEW_CONNECTION -3 //When this is the descriptor, it means somebody joined the host.
#define DESCRIPTOR_CODE_FOR_LOST_CONNECTION -4 //When this is the descriptor, it means somebody left the host.
#define BROADCAST_DESCRIPTOR 0 //When a client sends a message to that descriptor number, it means "to everyone".

#define INVALID_PORT -1
#define DISPLAY_TCPCLIENT_DEBUG_MESSAGES false

#define TCP_MESSAGE_SIZE 96 // Fixed size of a fragment that will be sent through the network.
#define UDP_MESSAGE_SIZE 48

/**
 * @brief This class represents a single client connected to a server through a TCP socket.
 * It is a class that can only be called by the Network or the TCPServer.
 */

class TCPClient : public QObject
{
    Q_OBJECT
public:
    TCPClient(QObject *parent);
    ~TCPClient();
    void connectToHost(QString ip, qint32 port = 0);
    bool write(QByteArray msg);
    bool writeUDP(QByteArray msg);
    bool initializeServerSideClient(qint32 socketDescriptor);
    bool disconnectFromHost();
    qint32 getSocketDescriptor();

signals:
    void networkMessageReceived(TCPClient *sender, qint32 descriptor, QByteArray message);
    void errorWarning(QString errorMessage);
    void clientDisconnected(TCPClient *client, bool wasServerClosed = false);
    void clientConnected(bool success);
    void someoneConnectedToHost(qint32 descriptor);
    void someoneDisconnectedFromHost(qint32 descriptor);

private slots:
    void disconnected();
    void connected();
    void socketError(QAbstractSocket::SocketError);
    void bytesWritten(qint64 bytes);
    void readyRead();

private:
    void setSocketDescriptor(qint32 descriptor);
    void declareAndConnectSockets();

    QTcpSocket *tcpSocket;
    QUdpSocket *udpSocket;

    bool isConnectedToHost;
    bool isServerSideClient;
    QString hostAddress;
    qint32 hostPort;
    qint32 socketDescriptor; //Works as a client unique id
    
};

#endif // LOCALCLIENT_H
