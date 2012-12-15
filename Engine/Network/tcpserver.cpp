#include "tcpserver.h"

TCPServer::TCPServer(QObject *parent)
{
    /*
     * Make sure that the class instantiating the TCPClient is from
     * an expected type. Only the Network can call it
     * (to prevent errors from happening):
     */
    if (!parent->inherits("Network")) {
        qCritical() << "[Server] Error: Only the Network can contact this class.";
        return;
    }

     hostUDP = new QUdpSocket(); //A host has also a UDP socket.
     hostDescriptor = INVALID_HOST_DESCRIPTOR;
     connect(hostUDP,SIGNAL(readyRead()),this,SLOT(messageReceivedUDP()), Qt::QueuedConnection);
}

TCPServer::~TCPServer()
{
    delete hostUDP;
}

bool TCPServer::startServer(qint32 port)
{
    if (isListening()) {
        emit errorWarning(tr("The server is already online. Operation cancelled."));
        return false;
    }

    if ( !listen(QHostAddress::Any, port) ) {
        emit errorWarning(tr("Server could not start."));
        return false;
    }

    if (!hostUDP->bind(QHostAddress::Any, serverPort())) {
        emit errorWarning(tr("Server could not bind UDP."));
        return false;
    }

    //qDebug() << "A new server has started.";
    return true;
}

bool TCPServer::closeServer()
{
    this->close();
    hostUDP->close();
    if (clientList.isEmpty()) return true;

    qDebug() << "Server closed. Clients dropped:" << clientList.size();
    foreach (TCPClient *client, clientList) {
        client->disconnectFromHost();
        delete client;
    }
    clientList.clear();
    return true;
}

/**
  This function is called when a user connects to the server:
  */
void TCPServer::incomingConnection(int handle)
{
    TCPClient *client = new TCPClient(this);
    client->initializeServerSideClient(handle);
    if (hostDescriptor == INVALID_HOST_DESCRIPTOR) 
    	hostDescriptor = handle; //host is the first to join the server.
    connect( client, SIGNAL(networkMessageReceived(TCPClient*, qint32, QByteArray)), this, SLOT(messageReceived(TCPClient*,qint32,QByteArray)), Qt::QueuedConnection);
    connect( client, SIGNAL(clientDisconnected(TCPClient*)), this, SLOT(clientDisconnected(TCPClient*)), Qt::QueuedConnection);
    
    /* Add the new client to the hash table that stores the list of clients connected: */
    clientList[handle] = client;
    
    //qDebug() << "[Server] Client" << QByteArray::number(client->getSocketDescriptor()) << "has connected. Total clients now:" << clientList.size();

    /* Emit a signal to the person hosting the server: */
    emit newClient(handle);
}

void TCPServer::broadcastMessage(QByteArray msg)
{
    msg.resize(TCP_MESSAGE_SIZE);

	/*
	 * Send a message to everyone connected to the server:
	 */
    foreach (TCPClient *client, clientList)
        client->write(msg);
}

void TCPServer::sendMessageThatSomeoneConnected(qint32 descriptor)
{
    /*
     * This function is not being used because so far there is
     * no need to know if someone has connected. This will be
     * revealed later when the user sends a message to the host
     * saying "hi"
     * But leave it here just in case.
     */
    QByteArray message;
    QDataStream networkStream(&message, QIODevice::WriteOnly);
    networkStream << (qint32) DESCRIPTOR_CODE_FOR_NEW_CONNECTION;
    networkStream << (qint32) descriptor;
    broadcastMessage(message);
}

TCPClient *TCPServer::searchClient(qint32 descriptor)
{
    return clientList.value(descriptor, NULL);
}

void TCPServer::messageReceivedUDP()
{
    //Although the message was received as UDP, it will be sent to the client as TCP.

    while (hostUDP->pendingDatagramSize() >= UDP_MESSAGE_SIZE) {
        QByteArray buffer;
        buffer.resize( UDP_MESSAGE_SIZE );
        QHostAddress sender;
        quint16 senderPort;
        hostUDP->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);
        QDataStream networkStream(buffer);
        qint32 to;
        QByteArray message;
        networkStream >> to;
        networkStream >> message;
        sendMessage(DESCRIPTOR_CODE_FOR_HOST, to, message);
    }
}

void TCPServer::messageReceived(TCPClient *from, qint32 to, QByteArray message)
{
    if (from == NULL)
        qCritical() << "[Server] Received network message from NULL sender! Message ignored.";
    else sendMessage(from->getSocketDescriptor(), to, message);
}

void TCPServer::clientDisconnected(TCPClient *client)
{
    //Send message to everyone about client disconnected:
    QByteArray message;

    QDataStream networkStream(&message, QIODevice::WriteOnly);
    networkStream << (qint32) DESCRIPTOR_CODE_FOR_LOST_CONNECTION;
    networkStream << client->getSocketDescriptor();
    broadcastMessage(message);

    clientList.remove( client->getSocketDescriptor() );

    //qDebug() << "Client" << QByteArray::number(client->getSocketDescriptor()) << "was disconnected. Total clients now:" << clientList.size();
    delete client;
}

void TCPServer::sendMessage(qint32 from, qint32 to, QByteArray message)
{
    QByteArray newMessage;
    TCPClient *client;
    QDataStream networkStream(&newMessage, QIODevice::WriteOnly);

    networkStream << from; //Insert who is sending the message.
    networkStream << message;

    if (from > 0) { //If the message is coming from a client, make sure it is from a registered client:
        client = searchClient(from);
        if (client == NULL) {
            qWarning() << "Message received to" << to << ", but client" << from << "could not be found. Message ignored.";
            return;
        }
    }

    if (to == BROADCAST_DESCRIPTOR) //Send to everyone:
            broadcastMessage(newMessage);
    else {
        if (to == DESCRIPTOR_CODE_FOR_HOST) {
            //Send to the host user, which is the first user in the room:
            client = clientList[hostDescriptor];
            //qDebug() << "Message to host. Being redirect to user " << destiny->getSocketDescriptor();
        } else {
            //Search who to send to:
            client = searchClient(to);
            if (client == NULL) {
                qWarning() << "Message received to" << to << ", but client could not be found. Message ignored.";
                return;
            } else {
                //qDebug() << "[Host] Specific message sent to" << to << " (size: " << message.size() << ").";
            }
        }
        newMessage.resize(TCP_MESSAGE_SIZE);
        //Send message:
        client->write(newMessage);
    }
}
