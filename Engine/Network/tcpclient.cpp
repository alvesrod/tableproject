#include "tcpclient.h"

TCPClient::TCPClient(QObject *parent)
{
    /*
     * Make sure that the class instantiating the TCPClient is from
     * an expected type. Only the Network or the TCPServer can call it
     * (to prevent errors from happening):
     */
    if ( (!parent->inherits("Network")) && (!parent->inherits("TCPServer")) ) {
        qCritical() << "[Client] Error: Only the "
                    << "Network or TCPServer can contact this class.";
        return;
    }

    isConnectedToHost = false;
    isServerSideClient = false;
    tcpSocket = NULL;
    udpSocket = NULL;
    socketDescriptor = -1;
    hostPort = INVALID_PORT;
}

TCPClient::~TCPClient()
{
    delete tcpSocket;
    delete udpSocket;
}

void TCPClient::connectToHost(QString ip, qint32 port)
{
    if (isConnectedToHost) {
        emit errorWarning(tr("Trying to connect while already connected."));
        emit clientConnected(false);
    }
    hostAddress = ip;
    hostPort = port;
    declareAndConnectSockets();
    tcpSocket->connectToHost(ip, port);
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(connected()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
}

void TCPClient::socketError(QAbstractSocket::SocketError)
{
    emit errorWarning(tr("Error. ") + tcpSocket->errorString());
    emit clientConnected(false);
}

void TCPClient::connected()
{
    isConnectedToHost = true;
    socketDescriptor = tcpSocket->socketDescriptor();
    if (DISPLAY_TCPCLIENT_DEBUG_MESSAGES)
        qDebug() << "[ Client" << socketDescriptor << "] Connected to " << hostAddress << ":" << hostPort;
    emit clientConnected(true);
}

void TCPClient::setSocketDescriptor(qint32 descriptor)
{
    tcpSocket->setSocketDescriptor(descriptor);
    socketDescriptor = descriptor;
}

/**
  Called by the server:
*/
bool TCPClient::initializeServerSideClient(qint32 socketDescriptor)
{
    if (isConnectedToHost) {
        emit errorWarning(tr("Client cannot be initialized because it is already connected to the host"));
        return false;
    }

    declareAndConnectSockets();
    setSocketDescriptor(socketDescriptor);
    if (DISPLAY_TCPCLIENT_DEBUG_MESSAGES)
        qDebug() << "Server-side client" << socketDescriptor << "initialized.";
    isServerSideClient = true;
    return true;
}

bool TCPClient::disconnectFromHost()
{
    if ( (!isConnectedToHost) && (!isServerSideClient) )
        return false;

    tcpSocket->disconnectFromHost();
    return true;
}

qint32 TCPClient::getSocketDescriptor()
{
    return socketDescriptor;
}

bool TCPClient::write(QByteArray msg)
{
    if ( (!isConnectedToHost) && (!isServerSideClient) ) {
        qWarning() << "[Client] Trying to write when not connected to a host.";
        return false;
    }
    if (tcpSocket->state() == QAbstractSocket::UnconnectedState) return false;

    qint32 dataWritten = tcpSocket->write(msg);
    if (dataWritten != msg.size()) {
        qWarning() << "[Client] Wrote" << dataWritten << "bytes. It should have written" << msg.size();
        return false;
    }
    return true;
}

bool TCPClient::writeUDP(QByteArray msg)
{
    if (!isConnectedToHost) {
        qWarning() << "[Client] Trying to send UDP message when not connected to a host.";
        return false;
    }
    udpSocket->writeDatagram( msg , QHostAddress(hostAddress), hostPort );
    return true;
}

void TCPClient::disconnected()
{
    if (DISPLAY_TCPCLIENT_DEBUG_MESSAGES)
        qDebug() << "Client" << getSocketDescriptor() << "was disconnected from the server.";
    isConnectedToHost = false;
    hostAddress = "";
    emit clientDisconnected(this, (tcpSocket->error() == QAbstractSocket::RemoteHostClosedError));
}

void TCPClient::bytesWritten(qint64 bytes)
{
    if ( (!isServerSideClient) && (DISPLAY_TCPCLIENT_DEBUG_MESSAGES) )
        qDebug() << "[Network] Client" << socketDescriptor << "wrote" << bytes << "bytes.";
}

void TCPClient::readyRead()
{
    while (tcpSocket->bytesAvailable() >= TCP_MESSAGE_SIZE) {
        QByteArray message = tcpSocket->read(TCP_MESSAGE_SIZE);
        QDataStream networkStream( message );
        qint32 descriptor;
        networkStream >> descriptor;

        switch(descriptor) {
            case DESCRIPTOR_CODE_FOR_NEW_CONNECTION:
                networkStream >> descriptor;
                emit someoneConnectedToHost(descriptor);
                break;
            case DESCRIPTOR_CODE_FOR_LOST_CONNECTION:
                networkStream >> descriptor;
                emit someoneDisconnectedFromHost(descriptor);
                break;
            default :
                networkStream >> message;
                if (message.size() != 0)
                    emit networkMessageReceived(this, descriptor, message);
        }
    }
}

void TCPClient::declareAndConnectSockets()
{
    //Declare TCP and UDP sockets:
    if (tcpSocket) delete tcpSocket;
    if (udpSocket) delete udpSocket;
    tcpSocket = new QTcpSocket();
    udpSocket = new QUdpSocket();

    connect (tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()), Qt::QueuedConnection);
    connect (tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::QueuedConnection);
    connect (tcpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)), Qt::QueuedConnection);
}

