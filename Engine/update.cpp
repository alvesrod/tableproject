#include "update.h"

Update::Update(QWidget *parent) :
    QWidget(parent)
{
    versionNumber = "";
    versionDetails = "";
    versionLocation = "";
    download = NULL;
}

/**
  * Request update is called by the Main Menu when there is an update.
  */
void Update::requestUpdate(QString newVersion, QString details, QString folderLocation)
{
    /* Check if the app is already updated: */
    if (newVersion == Engine::getAppVersion()) {
        qDebug() << "[Update] No need to update the app because it is already updated.";
        return;
    }

    versionNumber = newVersion;
    versionDetails = details;
    versionLocation = folderLocation;

    /* Ask the user if he wants to update the app: */
    askToUpdateApp();
}

/**
  * Displays a popup asking the user if he wants to update the app.
  * If yes, call the updateApp() function.
  */
void Update::askToUpdateApp()
{
    /* Prepare dialog: */
    QMessageBox *msgBox = new QMessageBox(QMessageBox::Information, tr("New version!"),
                                          tr("There's a new version available."),
                                          QMessageBox::Apply | QMessageBox::Ignore, this->parentWidget());
    msgBox->setDefaultButton(QMessageBox::Apply);
    msgBox->setTextFormat(Qt::RichText);
    msgBox->setInformativeText("<b>" + tr("Current version:") + "</b> " + Engine::getAppVersion() + "<br><b>" +
                               tr("New version:") + "</b> " + versionNumber);

    /* Convert from HTML to QString: */
    QTextDocument document;
    document.setHtml(versionDetails);
    versionDetails = document.toPlainText();

    /* Add the version details to the message box: */
    msgBox->setDetailedText(versionDetails);

    int reply = msgBox->exec();

    /* If the user wants to update the app, call updateApp(): */
    if (reply == QMessageBox::Apply)
        updateApp();
}

/**
  * Update the application.
  */
void Update::updateApp()
{
    qDebug() << "Updating to version " << versionNumber << " at location " << versionLocation;

    if(!download) {
        download = new Downloader(versionLocation, this);

        /* The download class has the power of closing the application: */
        connect(download, SIGNAL(signalClose()), this, SLOT(closeApp()));
    }
    download->download();
}


void Update::closeApp()
{
    emit signalClose();
}
