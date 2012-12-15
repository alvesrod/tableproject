#include "updater.h"
#include "ui_updater.h"

Updater::Updater(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Updater)
{
    /*
     * Initialize the graphic user interface of Updater.
     */
    ui->setupUi(this);

    /* Indicates that the window has no borders: */
    setWindowFlags(Qt::FramelessWindowHint);
}

Updater::~Updater()
{
    delete ui;
}

/**
  * Manage the process of update.
  */
void Updater::startUpdate()
{
    qDebug() << "Updating application...";
    show();
    getAppDir();
    initFileList();

    QTimer::singleShot(TIMER_TO_START, this, SLOT(startUpdating()));
}

void Updater::startUpdating()
{
    replaceFiles();
}

/**
  * Return the directory which the current application is running
  * in. Typcially it is the path of the temp folder inside the
  * directory where the application is installed.
  */
void Updater::getAppDir()
{
    /*
     * The current path where the Updater is running is the temp
     * directory which is inside the main directory where the
     * application is installed.
     */
    mainPath = getMainPath();

    tempPath = getCurrentPath();
}

/**
  * Initialize two lists with the names of all the files inside
  * the current directory (where the application is installed)
  * and the temp directory (in the current directory).
  */
void Updater::initFileList()
{
    /*
     * Get all the file names of the files of old versions.
     */
    fileList = getFileList(mainPath);


    /*
     * Get all the file names of the downloaded files.
     */
    updateList = getFileList(tempPath);
}

/**
  * Check each file name of downloaded files with the file names
  * of the files of old versions. If any two fiels share the
  * same name, remove the old one, and move the new file from
  * the temp directory to the current directory to replace.
  */
void Updater::replaceFiles()
{
    foreach(QString update, updateList)
    {
        QString updateName = QString("%1").arg(update);

        /*
         * Check if there are two files share the same name.
         */
        bool allFilesAreRemoved;
        int trials = LIMIT;

        do {
            allFilesAreRemoved = true;
            foreach(QString file, fileList)
            {
                QString fileName = QString("%1").arg(file);
                if(fileName == updateName)
                {
                    QDir mainDir(mainPath);
                    qDebug() << "Removing " << fileName;
                    if(!mainDir.remove(fileName))
                    {
                        qDebug() << "File " << fileName << " could not be removed.";
                        QMessageBox::information(this, tr("File is being used"),
                                             tr("Please, close the file")
                                             + "<b> " + fileName + " </b> " + tr("to update this application."));
                        allFilesAreRemoved = false;
                    }
                }
            }
        trials--;
        } while ( (!allFilesAreRemoved) || (trials <= 0) );

        QString updateFileName = tempPath;
        updateFileName.append("/" + updateName);

        QString newFileName = mainPath;
        newFileName.append("/" + updateName);

        qDebug() << "COPYING FILE " << updateName << " FROM " << tempPath << " TO " << newFileName;

        QDir dir;
        QFile updateFile(updateFileName);

        if (updateName.contains("/"))
            dir.mkpath(mainPath + "/" + updateName.left(updateName.lastIndexOf("/")));

        /*
         * Copy the new file into the main directory of the application.
         * The old files would be removed later by Application when it starts.
         */
        if (!updateFile.copy(newFileName)) {
            QMessageBox::warning(this, tr("Could not copy files!"),
                                 "<b>" + tr("The new version could not be copied.")
                                 + "</b><br> " + tr("Tried to copy file") + " "
                                 + updateFileName + " " + tr("to location")
                                 + " " + newFileName);
            return;
        }
    }

    restartApp();
}

/**
  * Call the Application to start and terminate the Updater
  * application.
  */
void Updater::restartApp()
{
    QString filename = QFileInfo( QCoreApplication::applicationFilePath() ).fileName();

    /*
     * Call the Application to start.
     */
    QProcess *process = new QProcess(this);
    process->setProcessEnvironment( QProcessEnvironment::systemEnvironment() );
    process->start("\"" + QDir::toNativeSeparators(mainPath + "/" + filename) + "\"");

    /*
     * Terminate the Updater itself.
     */
    QTimer::singleShot(100, this, SIGNAL(signalClose()));
}

/**
  * Return the path where the application is installed.
  */
QString Updater::getMainPath()
{
    /*
     * Get the path of the parent directory of the current.
     */
    QString path = getCurrentPath();
    int index = path.lastIndexOf("/");
    path.remove(index, path.size());

    return path;
}

/**
  * Return the path where the Updater is running.
  */
QString Updater::getCurrentPath()
{
    return QCoreApplication::applicationDirPath();
}

/**
  * Return a list with all the file names of the files
  * which are the in the given path.
  */
QStringList Updater::getFileList(QString path)
{
    QStringList list;
    recurseAddDir(QDir(path), list);
    return list;
}

/**
  * Method to grab all the files in a directory.
  */
void Updater::recurseAddDir(QDir d, QStringList & list, QString localDir) {
    QStringList qsl = d.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
    foreach (QString file, qsl) {
        QFileInfo finfo(QString("%1/%2").arg(d.path()).arg(file));
        if (finfo.isSymLink())
            return;
        if (finfo.isDir()) {
            QString dirname = finfo.fileName();
            QDir sd(finfo.filePath());
            recurseAddDir(sd, list, localDir + dirname + "/");
        } else
            list << localDir + finfo.fileName();
    }
}
