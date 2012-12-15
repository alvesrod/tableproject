#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QDialog>
#include <QtNetwork>
#include <QDataStream>
#include <QMessageBox>
#include "engine.h"

namespace Ui {
// The downloader view.
class Downloader;
}

#define TEMP_FOLDER "new_version" //Name of the folder where the files will be stored temporarilly.
#define INDEX_FILE "index.txt" //File on the webserver with information about the files.

/**
 * @brief The Downloader class is called when there's a new version
 * availale and the user wants to download this new version. This
 * class downloads it (while giving to the user a feedback of how
 * much is being downloaded). After the new version was downloaded,
 * this class will close this application to open another one that
 * will replace the app files (in order to update every file).
 */

class Downloader : public QDialog
{
    Q_OBJECT

public:
    explicit Downloader(QString path, QWidget *parent = 0);
    ~Downloader();

    /** Used to start the download: */
    void download();

signals:
    /**
      * Send a signal asking to close the application.
      */
    void signalClose();

protected:

    /** Retranslates the app if its language changed: */
    void changeEvent(QEvent* event);

private slots:
    void on_restartButton_clicked();
    void replyFinished(QNetworkReply *reply);
    void updateDataReadProgress(qint64 bytesRead,qint64 totalBytes);
    void updateDataDownloadProgress();
    void downloadFinished(QNetworkReply *reply);

private:

    /**
      * Send the request to a server to collect the information of a
      * target file with sepcific url.
      */
    void startRequest(QUrl url);

    /**
      * Read the index file to obtain the list of all the files
      * which are required to download.
      */
    void loadIndex();

    /**
      * Recurrence download manager
      */
    void downloadManager();

    /*
     * Basic graphic user interface of Updater.
     */
    Ui::Downloader *ui;

    /*
     * Network access manager for the target files.
     */
    QNetworkAccessManager *manager;

    /*
     * Network access manager for the index file.
     */
    QNetworkAccessManager *indexManager;

    /*
     * Url for every single file.
     */
    QUrl url;

    /*
     * File pointer for every single file.
     */
    QFile *file;

    /*
     * The path where the file is saved on the server.
     */
    QString serverPath;

    /*
     * The path where the file will be saved in the local machine.
     */
    QString localPath;

    /*
     * The list of files where are requested to download.
     */
    QStringList fileList;

    /*
     * The list of sizes of each file from the previous list.
     * The order of the sizes matches that of the file names.
     */
    QStringList fileSizeList;

    /*
     * The number of all the files which are downloading.
     */
    int totalFile;

    /*
     * The number of files which have completed downloading.
     */
    int doneFile;

    /*
     * Stores the total of bytes to be downloaded (for the loading bar):
     */
    qint64 totalBytesToDownload;

    /*
     * Stores the total of bytes downloaded (for the loading bar):
     */
    qint64 totalBytesDownloaded;

    /*
     * The size in bytes for the last file downloaded.
     */
    qint64 lastFileSize;

    /*
     * Stores the executable name. The name of this app:
     */
    QString filename;

};

#endif // DOWNLOADER_H
