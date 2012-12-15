#include "network.h"

Network::Network(QObject *parent)
{
	/*
	 * Make sure that the class instantiating the Network is from
	 * an expected type. Only 1 class can call the Network
	 * (to prevent errors from happening):
	 */
    if (!parent->inherits(CLASS_THAT_CAN_CONTACT_NETWORK)) {
        qCritical() << "[Network] Error: Only the "
                    << CLASS_THAT_CAN_CONTACT_NETWORK << " can contact the Network";
        return;
    }
    
    /*
     * The client being NULL means that you are not connected:
     */
    client = NULL;
    
    /*
     * The server being NULL means that you are not the server:
     */
    server = NULL;

    /* The timer will put a delay between large messages: */
    connect( &periodicMessagesTimer, SIGNAL(timeout()), this, SLOT(periodicTime()));
}

Network::~Network() //Destructor
{
    if (client != NULL) {
        delete client;
        client = NULL;
    }

    if (server != NULL) {
        delete server;
        server = NULL;
    }
}

void Network::run() //Required for the QThread
{
    exec();
}

/**
 * Emit a signal when someone connects or disconnects.
 * This signal is received even when you are just a client.
 */
void Network::someoneDisconnectedFromHost(qint32 descriptor)
{
    emit someoneDisconnected(descriptor);
}

void Network::someoneConnectedToHost(qint32 descriptor)
{
    emit someoneConnected(descriptor);
}

void Network::clientConnected(bool success)
{
    emit connectedToServer(success);
}

/**
 * Instantiate a new server and delete the old one.
 * Connect all signals received from the server to this class.
 */
void Network::refreshServer()
{
    if (server != NULL) {
        delete server;
        server = NULL;
    }
    server = new TCPServer(this);
    connect (server, SIGNAL(errorWarning(QString)), this, SLOT(errorWarning(QString)), Qt::QueuedConnection);
    connect (server, SIGNAL(newClient(qint32)), this, SLOT(someoneConnectedToHost(qint32)), Qt::QueuedConnection);
}

/**
 * Instantiate a new client and delete the old one.
 * Connect all signals received from the client to this class.
 */
void Network::refreshClient()
{
    if (client != NULL) {
        delete client;
        client = NULL;
    }
    client = new TCPClient(this);
    connect (client, SIGNAL(networkMessageReceived(TCPClient*, qint32, QByteArray)), this, SLOT(networkMessageReceived(TCPClient*, qint32, QByteArray)), Qt::QueuedConnection);
    connect (client, SIGNAL(errorWarning(QString)), this, SLOT(errorWarning(QString)), Qt::QueuedConnection);
    connect (client, SIGNAL(clientDisconnected(TCPClient*, bool)), this, SLOT(userDisconnected(TCPClient*, bool)), Qt::QueuedConnection);
    connect (client, SIGNAL(someoneConnectedToHost(qint32)), this, SLOT(someoneConnectedToHost(qint32)), Qt::QueuedConnection);
    connect (client, SIGNAL(someoneDisconnectedFromHost(qint32)), this, SLOT(someoneDisconnectedFromHost(qint32)), Qt::QueuedConnection);
    connect (client, SIGNAL(clientConnected(bool)), this, SLOT(clientConnected(bool)), Qt::QueuedConnection);
}

/**
 * Starts a server where other can join through the internet.
 * You need a port. Emits a signal "serverStarted" saying whether
 * or not you were able to connect to the server.
 */
void Network::startNewServer(qint32 port)
{
    refreshServer(); //Create a new server.
    bool result = server->startServer(port);
    emit serverStarted( result , server->serverPort() );
}

void Network::closeServer()
{
    if  ( (server == NULL) || (!server->isListening()) ) 
    	emit serverClosed(true); //If the server is not opened, pretend it was closed successfully.
    else emit serverClosed( server->closeServer() );
}

void Network::disconnectFromServer()
{
    if (client == NULL) emit disconnectedFromServer(true);
    else {
        bool result = client->disconnectFromHost();
        emit disconnectedFromServer(result);
    }
}

/** 
 * Join a server on the internet (or on the local network):
 * You join it based on the ip and port of the server.
 * connectToHost() in the client will emit a signal
 * warning if the connection was successful.
 */
void Network::connectToServer(QString ip, qint32 port)
{
    refreshClient();
    client->connectToHost(ip, port);
}

/**
 * Send a message to the server. The message is a
 * QByteArray. You send to the user "to" where "to" is
 * an integer with the socket descriptor of the user.
 * You can send TCP or UDP messages.
 * This function emits a signal "messageSent(bool)"
 * saying whether or not the message was sent.
 */
void Network::sendMessageToServer(QByteArray message, qint32 to, bool TCP)
{
    if (client == NULL) {
        qWarning() << "[Network] Message ignored because client is not connected.";
        emit messageSent(false);
    }
    else {
        if ( (to < 0) && (to != DESCRIPTOR_CODE_FOR_HOST) ) {
            qWarning() << "[Network] Tried to send a message to an invalid user.";
            emit messageSent(false);
            return;
        }
        if (message.size() == 0) {
            qWarning() << "[Network] Message ignored because the message is empty.";
            emit messageSent(false);
            return;
        }

        /* Compress the message to make it smaller: */
        QByteArray compressedMessage = qCompress(message);

        if (compressedMessage.size() > MAX_TOTAL_SIZE) {
            qWarning() << "[Network] Tried to send a message that is too big! Size:"
                       << compressedMessage.size() << "Limit:" << MAX_TOTAL_SIZE << "bytes.";
            emit messageSent(false);
            return;
        }

        /* Get the size of the packages: */
        const int MESSAGE_SIZE = (TCP) ? (TCP_MESSAGE_SIZE-13) : (UDP_MESSAGE_SIZE-13);
        //-13 because the header adds 13 bytes to the message.

        bool lastFragment = false;

        /* While there are still message to be sent: */
        while (compressedMessage.size() > 0) {

            /*
             * Add to the package the socket descriptor of who the message is going to,
             * so that when the server opens the package, it will know who to
             * send the message to:
             */
            QByteArray newMessage, fragment;
            QDataStream networkStream(&newMessage, QIODevice::WriteOnly);

            /* Insert in the package who the message is going to: */
            networkStream << to;

            QDataStream fragmentStream(&fragment, QIODevice::WriteOnly);

            /* Check if this will be the last fragment of message: */
            lastFragment = (compressedMessage.size() <= MESSAGE_SIZE);

            /* Let the receiver knows if this was the last fragment: */
            fragmentStream << lastFragment;

            /* Get the firts chunk of bytes and make it a fixed size: */
            QByteArray chunk = compressedMessage.left(MESSAGE_SIZE);
            chunk.resize(MESSAGE_SIZE);

            /* Add the first chunk of bytes into the stream and remove it: */
            fragmentStream << chunk;
            compressedMessage.remove(0, MESSAGE_SIZE);

            /* Insert the message in the stream: */
            networkStream << fragment;

            /* Add the message to a queue. The message will be sent eventually: */
            packageQueue << newMessage;
            lastFragmentQueue << lastFragment;
            tcpQueue << TCP;
        }

        /* Start the queue timer: */
        if (!periodicMessagesTimer.isActive())
            periodicMessagesTimer.start(MILLISECONDS_PACKAGE_DELAY);
    }
}

void Network::writeMessageInSocket(bool TCP, QByteArray newMessage, bool lastFragment)
{
    bool result = false;
    if (TCP)
        result = client->write(newMessage); //TCP Message.
    else
        result = client->writeUDP(newMessage); //UDP Message.

    if ( (result) && (lastFragment) )
        emit messageSent(true);
}

void Network::isConnectedToServer()
{
    if (server == NULL) emit connectedToServer(false);
    else emit connectedToServer( server->isListening() );
}

void Network::networkMessageReceived(TCPClient *to, qint32 from, QByteArray message)
{
    if (to != client) {
        qWarning() << "Trying to receive a message that was not sent for you!";
        return;
    }

    bool lastFragment;
    QByteArray compressedMessage;
    QDataStream fragmentStream(message);
    fragmentStream >> lastFragment;
    fragmentStream >> compressedMessage;

    /* Add to the end of the fragment (composing the message): */
    fragments[from].append(compressedMessage);

    if (lastFragment) {

        /*
         * Emit a signal warning that a message arrived. The message is
         * uncompressed because it was compressed before.
         * The parent of this class is the one likely to receive this signal.
         */
        /* The message is uncompressed because it was compressed before: */
        emit networkMessage(from, qUncompress(fragments[from]));

        /* Cleam the fragment byte array: */
        fragments[from].clear();
    }
    

}

void Network::errorWarning(QString error)
{
    qCritical() << "[Network Error] " << error;
    emit errorMessage(error);
}

/**
 * Called when YOU are disconnected from the server:
 */
void Network::userDisconnected(TCPClient *clientDisconnected, bool wasServerClosed)
{
    if (clientDisconnected != client) {
        qWarning() << "Received an unexpected signal from a different client.";
        return;
    }

    /* Check if the disconnection is because the host closed the room: */
    if (wasServerClosed)
        emit serverClosed();
    else
        emit disconnectedFromServer(); //Any other reason
}

/**
 * Called frequently to send the messages waiting in the queue.
 */
void Network::periodicTime()
{
    int packagesSent = 0;

    /* Send the message through the network: */
    while ( (!lastFragmentQueue.isEmpty()) && (packagesSent < TOTAL_PACKAGES_SENT_AFTER_DELAY) ) {
        writeMessageInSocket(tcpQueue.takeFirst(), packageQueue.takeFirst(), lastFragmentQueue.takeFirst());
        packagesSent++;
    }

    if (lastFragmentQueue.isEmpty()) {
        periodicMessagesTimer.stop();
        return;
    }
}


