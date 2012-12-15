#include "downloader.h"
#include "ui_downloader.h"

Downloader::Downloader(QString versionLocation, QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint),
    ui(new Ui::Downloader)
{
    /* Make sure the Engine is running: */
    if (!Engine::isRunning()) {
        qCritical() << "[ERROR] Engine class called, but Engine was not started!";
        return;
    }

    filename = QFileInfo( QCoreApplication::applicationFilePath() ).fileName();

    /*
     * Initialize class variables:
     */
    totalBytesToDownload = 0;
    totalBytesDownloaded = 0;
    totalFile = 0;
    doneFile = 0;

    /*
     * Initialize the downloader user interface.
     */
    ui->setupUi(this);
    ui->fileProgressBar->hide();
    ui->processProgressBar->hide();
    ui->restartButton->hide();

    /*
     * Initialize the network and files.
     */
    manager = new QNetworkAccessManager(this);
    indexManager = new QNetworkAccessManager(this);
    file = NULL;

    /*
     * Set up the paths for server and local machine.
     */
    serverPath = versionLocation;
    if(serverPath == NULL)
    {
        QMessageBox::warning(this, tr("ERROR"), tr("The URL path is invalid!"));
    }

    localPath = QCoreApplication::applicationDirPath();

    connect(indexManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));
    connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(downloadFinished(QNetworkReply*)));

    /*
     * Set the background color to white:
     */
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setPalette(pal);
}

Downloader::~Downloader()
{
    manager->deleteLater();
    delete ui;
}

/**
  * Start download process.
  */
void Downloader::download()
{
    /*
     * Initialize both progress bars.
     * Display both progress bars on the interface.
     */
    ui->fileProgressBar->setValue(0);
    ui->fileProgressBar->show();
    ui->processProgressBar->setValue(0);
    ui->processProgressBar->show();

    totalBytesToDownload = 0;
    totalBytesDownloaded = 0;

    /* Display the window to the user: */
    show();
    activateWindow();

    /*
     * Access the index file on the website to obtain the list
     * of all the files which need to be downloaded.
     */
    loadIndex();
}

/**
  * Read the index file to obtain the list of all the files
  * which are required to download.
  */
void Downloader::loadIndex()
{
    indexManager->get(QNetworkRequest(QUrl(serverPath + "/" + INDEX_FILE)));
}

/**
  * Start the request from the website.
  * Load the information of the downloading files.
  */
void Downloader::startRequest(QUrl url)
{
    QNetworkReply *reply = manager->get(QNetworkRequest(url));
    connect(reply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(updateDataReadProgress(qint64,qint64)));
}

/**
  * Load the list of files.
  * Start the loop of downloading.
  */
void Downloader::replyFinished(QNetworkReply *reply)
{
    /*
     * Split the text into list with each file and its size.
     */
    QString index = reply->readAll();
    fileList << index.split("\n",QString::SkipEmptyParts);

    /*
     * Make a path for the temp folder (if there is any):
     */
    QDir dir;
    QString tempFolderPath = localPath + "/" + TEMP_FOLDER;
    if (!dir.mkpath(tempFolderPath)) {
        qCritical() << "[Downloader] Could not create temporary folder!";
        return;
    }

    for (int i = fileList.size() - 1; i > 0; i-=2) {

        /*
         * Check if the file has already existed in the local machine.
         * If it is positive, then skip downloading this file.
         * Otherwise, download this file.
         */
        QString name = fileList.at(i-1);
        QString oldFileName = localPath + "/" + name;
        QFile oldFile(oldFileName);

        qint64 fileSize = fileList.at(i).toInt();

        /* Don't download a file again if you have one with the same size (except with the main file). */
        if ( (oldFile.exists()) && (oldFile.size() == fileSize) && (name != filename))
        {
            qDebug() << "FILE " << oldFileName << " TO BE DOWNLOADED ALREADY EXISTED. COPIED TO FOLDER.";

            /* Create a path in the temp directory: */
            if (name.contains("/"))
                dir.mkpath(tempFolderPath + + "/" + name.left(name.lastIndexOf("/")));

            /* Copy the local file to the temp directory instead of downloading it: */
            if (oldFile.copy(tempFolderPath + "/" + name)) {

                /*
                 * Remove the file from the list of files that will be downloaded.
                 * This is better done here than inside the manager because it gives
                 * an accurate feedback to the user of how much files he/she has to download.
                 */
                fileList.removeAt(i);
                fileList.removeAt(i-1);
            } else
                totalBytesToDownload += fileSize;
        }
        else
            totalBytesToDownload += fileSize;
    }

    /* Update the progress bar: */
    ui->processProgressBar->setMaximum(totalBytesToDownload);
    ui->processProgressBar->setValue(totalBytesDownloaded);

    reply->deleteLater();

    /*
     * Initialize the index of the file list.
     */
    totalFile = fileList.size()/2;
    doneFile = -1;


    /*
     * Start the downloading by the index of the list.
     */
    downloadManager();
}

/**
  * Recurrence download manager
  */
void Downloader::downloadManager()
{
    /*
     * Check if all the files have been completed.
     * If it is positive, reset the graphic user interface.
     */
    if(fileList.isEmpty())
    {
        /*
         * Clear the information display of downloading.
         * Enable the restart button to install the downloaded files.
         * Display the window again if the window was closed.
         */
        ui->infoLabel->setText(tr("Download complete."));
        setWindowTitle(tr("Application updated."));
        ui->FileLabel->hide();
        ui->ProcessLabel->hide();
        ui->fileProgressBar->hide();
        ui->processProgressBar->hide();
        ui->restartButton->show();
        QSound::play("Sound/appUpdated.wav");
        show();
        activateWindow();

        return;
    }

    /*
     * Update the overall downloading progress status.
     */
    ui->fileProgressBar->setValue(0);
    updateDataDownloadProgress();

    /*
     * Read the file name and file size.
     */
    QString fileName = fileList.takeFirst();
    QString fileSize = fileList.takeFirst();
    lastFileSize = fileSize.toInt();

    ui->FileLabel->setText(tr("Downloading File:") + " " + fileName);

    /*
     * Set up the path where the file is on the server.
     */
    QString filePath = serverPath + "/" + fileName;
    url.setUrl(filePath);

    QString localFileName = localPath + "/" + TEMP_FOLDER + "/";

    /*
     * Create a subdirectory in the temp directory if necessary.
     */
    QDir dir;
    if (fileName.contains("/"))
        dir.mkpath(localFileName + fileName.left(fileName.lastIndexOf("/")));

    localFileName += fileName;

    file = new QFile(localFileName,this);

    if(!file->open(QIODevice::WriteOnly))
    {
        qDebug() << "FILE " << localFileName << " OPEN ERROR";
        return;
    }

    /*
     * Start to make the request to the website with a
     * given url.
     */
    startRequest(url);
}

/**
  * Update the graphic status of the current downloading file.
  */
void Downloader::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    ui->fileProgressBar->setMaximum(totalBytes);
    ui->fileProgressBar->setValue(bytesRead);
    ui->processProgressBar->setValue(totalBytesDownloaded + bytesRead);
}

/**
  * Update the graphic status of the overall downloading progress.
  */
void Downloader::updateDataDownloadProgress()
{
    /*
     * Display the number of completed downloaded files
     * and the number of the total files in the list.
     */
    doneFile++;
    ui->ProcessLabel->setText(QString::number(doneFile) + " / " + QString::number(totalFile)
                              + " " + tr("file") + ((doneFile != 1)?tr("s downloaded."):tr(" downloaded.")));
    ui->processProgressBar->setValue(totalBytesDownloaded);
}

/**
  * Close the local file object.
  * Make the recurrence call for download manager.
  */
void Downloader::downloadFinished(QNetworkReply *reply)
{
    /*
     * Write the contents into the new created local file.
     */
    if(file)
    {
        file->write(reply->readAll());
    }

    /*
     * Close the local file.
     */
    file->flush();
    file->close();

    /*
     * Add the file size to the total of bytes downloaded:
     */
    totalBytesDownloaded += lastFileSize;

    /*
     * Recall the download for the next file recurrently.
     */
    downloadManager();
}

/**
  * Restart the application to install the update.
  */
void Downloader::on_restartButton_clicked()
{

    /* Start the application again with a flag to update the application: */
    QString applicationPath = localPath + "/" + TEMP_FOLDER "/" + filename;
    QStringList arguments;
    arguments << UPDATE_FLAG;
    QProcess *process = new QProcess(this);
    process->setProcessEnvironment( QProcessEnvironment::systemEnvironment() );
    process->start("\"" + QDir::toNativeSeparators(applicationPath) + "\"", arguments);

    /*
     * Emit a signal asking to close the application.
     */
    emit signalClose();
}


/** Retranslates the app if its language changed: */
void Downloader::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
