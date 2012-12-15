#ifndef FILETRANSFER_H
#define FILETRANSFER_H

#include <QObject>
#include <QHash>
#include <QTimer>
#include "roomcontroller.h"

#define PACKAGE_CHUNKS_BYTES 7000
#define MILLISECONDS_DELAY_BETWEEN_PACKAGES 2 //Milliseconds

/**
 * @brief The FileTransfer class deals with large network packages that are
 * too big to be sent as one single message. A class can send a network
 * package through this class or a file in the computer. Whoever is receiving
 * this large package will receive in pieces and be warned about its progress.
 * This class takes a timer between packages to allow transference of files
 * without disrupting with other network packages being sent. And this is a
 * thread with low priority (the network messages related to the action are
 * more important the the messages from a file being downloaded).
 */

class FileTransfer : public QThread
{
    Q_OBJECT
public:
    explicit FileTransfer(RoomController *roomController);

    /**
     ** The types of download messages:
     **/
    enum DownloadMessageTypes {
        DOWNLOAD_START,
        DOWNLOAD_STEP

        /*
         * YOU DEAL WITH THEM IN THE
         * roomButtonClicked() AND IN
         * THE MAIN WINDOW.
         */
    };

public slots:

    /** Send a large package to @param User */
    void sendPackage(QByteArray largePackage, QByteArray info, User *user = 0);

    /** Send a file stored at @param path to @param user: */
    void sendFile(QString filepath, QByteArray info, User *user = 0);

signals:
    /* Signals sent to the receiver alerting about the progress of the file being transfered: */
    void started(QByteArray info, qint32 size, User *user);
    void downloadProgress(QByteArray info, double percentage, User *user);
    void finished(QByteArray info, QByteArray package, User *user);
    void cancelled(QByteArray info, User *user);

    /* Signal sent to the network: */
    void sendDownloadMessageToServer(QByteArray message, User *to);
    
private slots:
    /* Called from the Room Controller when a message related to this class is received */
    void downloadMessageReceived(QByteArray message, User *user);

    /* Called from the Room Controller when a user leaves the room (could be a user uploading): */
    void someoneDisconnected(User* user);

    /* Called by the timer: */
    void periodicTime();

protected:
    void run();

private:
    void handleDownloadStep(QByteArray packageHash, QByteArray chunk, User *user);
    QByteArray packageHash(QByteArray package);
    QByteArray cryptPackage(QByteArray& package);
    QByteArray decryptPackage(QByteArray& package);
    QByteArray cryptHelper(QByteArray& package, const QString stringKey);

    QHash<QByteArray,QByteArray> packages;
    QHash<QByteArray,qint32> packageSizes;
    QHash<QByteArray,QByteArray> packageInfo;

    /* List of all users that you are downloading packages from: */
    QList<User*> sendersList;
    QList<QByteArray> hashList;

    QList<User*> usersQueue;
    QList<QByteArray> packageQueue;

    quint32 packageCount;

    /* The timer gives a break for other network messages to also be sent: */
    QTimer periodicMessagesTimer;

    RoomController *controller;
    
};

#endif // FILETRANSFER_H
