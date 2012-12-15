#ifndef UPDATE_H
#define UPDATE_H

#include <QObject>
#include <QMessageBox>
#include <QDir>
#include "Views/downloader.h"

class Downloader;

/**
  @brief This class receives a request to update the app
  and asks the user if he/she wants to update the app.
  If yes, the app will be updated.
  */


class Update : public QWidget
{
    Q_OBJECT
public:
    explicit Update(QWidget *parent = 0);
    void requestUpdate(QString newVersion, QString details, QString folderLocation);

signals:
    void signalClose(); //send a signal asking to close the application

private slots:
    /** Can be called to close the application: */
    void closeApp();

private:
    void askToUpdateApp();
    void updateApp();

    QString versionNumber;
    QString versionDetails;
    QString versionLocation;
    
    Downloader *download;
};

#endif // UPDATE_H
