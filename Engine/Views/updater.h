#ifndef UPDATER_H
#define UPDATER_H

#include <QWidget>
#include <QProcess>
#include <QStringList>
#include <QDir>
#include <QCoreApplication>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>

namespace Ui {
// The Update View
class Updater;
}

#define LIMIT 5 //Limit of trials to remove a file before exiting.
#define TIMER_TO_START 600 //Milliseconds to wait the old file to be closed to start removing.

/**
  * @brief The Updater class is called by the downloader class from
  * Application. It removes all the files of old versions, and then
  * moves the downloaded files of the new version into the main
  * directory from the folder 'temp' in order to replace the old
  * version of the application. After all the files have been updated,
  * the Updater class would close itself and call Application executable
  * to restart the application to apply the update.
  */

class Updater : public QWidget
{
    Q_OBJECT

public:
    explicit Updater(QWidget *parent = 0);
    ~Updater();

    /**
      * Manage the process of update. Call it to start updating.
      */
    void startUpdate();

    /**
      * Return the directory which the current application is running
      * in. Typcially it is the path of the temp folder inside the
      * directory where the application is installed.
      */
    void getAppDir();

    /**
      * Initialize two lists with the names of all the files inside
      * the current directory (where the application is installed)
      * and the temp directory (in the current directory).
      */
    void initFileList();

    /**
      * Check each file name of downloaded files with the file names
      * of the files of old versions. If any two fiels share the
      * same name, remove the old one, and move the new file from
      * the temp directory to the current directory to replace.
      */
    void replaceFiles();

    /**
      * Call the Application to start and terminate the Updater
      * application.
      */
    void restartApp();

    /**
      * Return the path where the application is installed.
      */
    QString getMainPath();

    /**
      * Return the path where the Updater is running.
      */
    QString getCurrentPath();

    /**
      * Return a list with all the file names of the files
      * which are the in the given path.
      */
    QStringList getFileList(QString path);

signals:
    /**
      * Send a signal asking to close the application.
      */
    void signalClose();

private slots:
    void startUpdating();

private:

    /**
      * Method to grab all the files in a directory.
      */
    void recurseAddDir(QDir d, QStringList & list, QString localDir = "");

    /*
     * Basic graphic user interface of Updater.
     */
    Ui::Updater *ui;

    /*
     * The path where the files of old versions are.
     */

    QString mainPath;
    /*
     * The path where the downloaded files are.
     */

    QString tempPath;

    /*
     * The list of file names of the files of old versions.
     */

    QStringList fileList;
    /*
     * The list of file names of the downloaded fiels.
     */
    QStringList updateList;

};

#endif // UPDATER_H
