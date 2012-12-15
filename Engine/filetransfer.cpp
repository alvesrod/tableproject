#include "filetransfer.h"

FileTransfer::FileTransfer(RoomController *roomController)
{
    /* Make sure the Engine is running: */
    if (!Engine::isRunning()) {
        qCritical() << "[ERROR] Engine class called, but Engine was not started!";
        return;
    }

    if (roomController == NULL) {
        qCritical() << "[ERROR] FileTransfer needs a Room Controller";
        return;
    }

    controller = roomController;

    connect(controller, SIGNAL(downloadMessageReceived(QByteArray,User*)),
                      this, SLOT  (downloadMessageReceived(QByteArray,User*)), Qt::QueuedConnection);

    connect(controller, SIGNAL(someoneDisconnectedFromServer(User*)),
                      this, SLOT(someoneDisconnected(User*)), Qt::QueuedConnection);

    connect(           this, SIGNAL(sendDownloadMessageToServer(QByteArray,User*)),
             controller, SLOT(sendDownloadMessageToServer(QByteArray,User*)), Qt::QueuedConnection);



    packageCount = 0;

    connect (&periodicMessagesTimer, SIGNAL(timeout()), this, SLOT(periodicTime()));

    /* Delete this thread when it is finished: */
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void FileTransfer::sendPackage(QByteArray largePackage, QByteArray info, User *user)
{

    QByteArray message, packageHash;

    QDataStream hashStream(&packageHash, QIODevice::WriteOnly);
    hashStream << QCryptographicHash::hash(largePackage,QCryptographicHash::Md5);
    hashStream << packageCount;
    largePackage = cryptPackage(largePackage);
    packageCount++;

    qint8 type = DOWNLOAD_START;
    qint32 size = largePackage.size();
    QDataStream networkStream(&message, QIODevice::WriteOnly);
    networkStream << packageHash;
    networkStream << type;
    networkStream << size;
    networkStream << info;
    emit sendDownloadMessageToServer(message, user);

    type = DOWNLOAD_STEP;

    int index = 0;

    while (largePackage.size() > 0) {

        QByteArray chunk;
        QDataStream networkStream(&chunk, QIODevice::WriteOnly);
        networkStream << packageHash;
        networkStream << type;

        /* Add the first chunk of bytes into the stream and remove it: */
        networkStream << largePackage.left(PACKAGE_CHUNKS_BYTES);
        largePackage.remove(0, PACKAGE_CHUNKS_BYTES);

        /*
         * Insert packages in the queue, but mixing them up with other package requests.
         * So that, all packages are sent simultaneously:
         */
        usersQueue.insert(index, user);
        packageQueue.insert(index, chunk);
        index+= 2;
        if (index > usersQueue.size())
            index = usersQueue.size();

    }
    if (!periodicMessagesTimer.isActive())
        periodicMessagesTimer.start(MILLISECONDS_DELAY_BETWEEN_PACKAGES);
}

void FileTransfer::sendFile(QString filepath, QByteArray info, User *user)
{
    QFile file(filepath);
    if(!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[FileTransfer] File to be sent could not be opened at location " << filepath;
        return;
    }
    sendPackage(file.readAll(), info, user);
    file.close();
}


void FileTransfer::downloadMessageReceived(QByteArray message, User *user)
{
    if (user == NULL) {
        qDebug() << "[FileTransfer] Received message from NULL user. Message ignored.";
        return;
    }

    QDataStream messageStream(message);
    qint8 type;
    qint32 size;
    QByteArray packageHash, package;
    messageStream >> packageHash;
    messageStream >> type;

    switch (type) {

    case DOWNLOAD_START:

        messageStream >> size;
        messageStream >> package;
        packageCount++;
        packages[packageHash] = QByteArray();
        packageSizes[packageHash] = size;
        packageInfo[packageHash] = package;
        sendersList << user;
        hashList << packageHash;
        emit started(package, size, user);
        break;

    case DOWNLOAD_STEP:
        messageStream >> package;
        handleDownloadStep(packageHash, package, user);
        break;

    default:
        qWarning() << "[FileTransfer] Error: Received message from unkown type.";
    }
}

void FileTransfer::someoneDisconnected(User *user)
{
    /* If you were sending packages to that user, stop: */
    qDebug() << "User " << user->getNickname() << "left. Size:" << usersQueue.size();
    for (int i = 0; i < usersQueue.size(); i++) {
        if (usersQueue.at(i) == user) {
            qDebug() << "REMOVED AT " << i;
            usersQueue.removeAt(i);
            packageQueue.removeAt(i);
        }
    }

    /* Check if you were receiving a package from a user that left: */
    for (int i = 0; i < sendersList.size(); i++) {
        if (sendersList.at(i) == user) {
            emit cancelled(packageInfo[hashList.at(i)], user);
            packages[hashList.at(i)].clear();
            packageInfo[hashList.at(i)].clear();
            sendersList.takeAt(i);
            hashList.takeAt(i);
        }
    }
}

void FileTransfer::periodicTime()
{
    if (usersQueue.isEmpty()) {
        periodicMessagesTimer.stop();
        return;
    }
    emit sendDownloadMessageToServer(packageQueue.takeFirst(), usersQueue.takeFirst());
}

void FileTransfer::run()
{
    exec();
}

void FileTransfer::handleDownloadStep(QByteArray packageHash, QByteArray chunk, User *user)
{
    if (!packages.contains(packageHash)) {
        qWarning() << "[FileTransfer] Error: Received an unexpected package chunk. ";
        return;
    }

    packages[packageHash].append(chunk);
    qint32 currentBytes = packages[packageHash].size();
    qint32 totalBytes = packageSizes[packageHash];

    qDebug() << "Received chunk from file-transfer. Size: " << chunk.size();

    if (currentBytes > totalBytes) {
        emit finished(packageInfo[packageHash], QByteArray(), user);
        packages[packageHash].clear();
        packageInfo[packageHash].clear();
        int index = hashList.indexOf(packageHash);
        hashList.takeAt(index);
        sendersList.takeAt(index);
        qWarning() << "[FileTransfer] Error: Received network Package from unexpected size.";
        return;
    }

    if (currentBytes == totalBytes) {
        qDebug() << "File is fully downloaded.";
        QByteArray package = decryptPackage( packages[packageHash]);
        emit finished(packageInfo[packageHash], package , user);
        int index = hashList.indexOf(packageHash);
        hashList.takeAt(index);
        sendersList.takeAt(index);
        packages[packageHash].clear();
        packageInfo[packageHash].clear();
        return;
    }
    emit downloadProgress(packageInfo[packageHash], ((currentBytes * 100.0) / (double) totalBytes), user);
}


QByteArray FileTransfer::cryptPackage(QByteArray &package)
{
    return qCompress( controller->crypt(package, QByteArray("This is not the key") ,true) );
}


QByteArray FileTransfer::decryptPackage(QByteArray &package)
{
    return controller->crypt(qUncompress(package) , QByteArray("This is not the key"), true);
}


