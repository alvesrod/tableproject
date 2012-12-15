#include "newroomdialog.h"
#include "ui_newroomdialog.h"

NewRoomDialog::NewRoomDialog(UserSettings *settings, QWidget *parent) :
    NewRoom(settings, parent),
    ui(new Ui::NewRoomDialog)
{
    ui->setupUi(this);

    /* Ask the superclass to prepare the view: */
    setupNewRoomUI();

    /* Create a new Room Description (using the Engine Factory to help): */
    roomDescription = dynamic_cast<RoomInfo*> (Engine::newRoomDescription(0));

    /* Subscribe to updates in the data, so that the updates can be instantly seen in the view: */
    connect( roomDescription, SIGNAL(dataUpdated()), this, SLOT(tableDataUpdated()) );
}

NewRoomDialog::~NewRoomDialog()
{
    delete ui;
}

void NewRoomDialog::on_startServerBtn_clicked()
{
    if (ui->roomNameEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Name"), tr("You need to give a name to your room."));
        return;
    }
    if (ui->roomTypeEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Type"),
                             tr("You need to set the type of room that you are hosting. Eg: Chess game"));
        return;
    }

    prepareRoomDescription();
    startNewRoom( roomDescription );
}

void NewRoomDialog::on_settingsBtn_clicked()
{
    openUserSettingsDialog();
}

/** Display the room description values in the GUI: */
void NewRoomDialog::tableDataUpdated()
{
    ui->portSpinBox->setValue( roomDescription->getRoomPort() );
    ui->roomNameEdit->setText( roomDescription->getRoomName() );
    ui->roomTypeEdit->setText( roomDescription->getRoomType() );
    ui->descriptionTextEdit->setPlainText(roomDescription->getRoomDescription());
    ui->maxUserSpinBox->setValue( roomDescription->getMaxNumberOfUsers() );
    ui->addImageComboBox->setCurrentIndex( roomDescription->getAddObjectPermission() );
    ui->removeImageComboBox->setCurrentIndex( roomDescription->getRemoveObjectPermission() );
    ui->changeTeamsComboBox->setCurrentIndex( roomDescription->getChangeTeamsPermission() );
    ui->hideImageComboBox->setCurrentIndex( roomDescription->getHideObjectPermission() );
    ui->lockImageComboBox->setCurrentIndex( roomDescription->getLockObjectPermission() );
    ui->banUsersComboBox->setCurrentIndex( roomDescription->getBanPermission() );
    ui->teamSortingComboBox->setCurrentIndex( roomDescription->getTeamSortingType() );
}

void NewRoomDialog::on_loadTableBtn_clicked()
{
    QString path = qApp->applicationDirPath() + "/" + ROOM_FOLDER_NAME;
    QString filePath = QFileDialog::getOpenFileName(this,tr("Open Table"), path,
                                                    tr("Table Files ") + "(*." + TABLE_FILE_TYPE + ")");
    if (filePath.isEmpty())
        return;

    if (! roomDescription->loadRoomFile(filePath) )
        QMessageBox::information(this, tr("Invalid File"), tr("The file loaded is invalid. Please, try a different one."));
}

void NewRoomDialog::prepareRoomDescription()
{
    /* Set up a room for the new server: */
    if ( ui->portCheckBox->isChecked() )
        roomDescription->setRoomPort( 0 );
    else
        roomDescription->setRoomPort( ui->portSpinBox->value() );

    roomDescription->setRoomName( ui->roomNameEdit->text() );
    roomDescription->setRoomType( ui->roomTypeEdit->text() );
    roomDescription->setRoomDescription( ui->descriptionTextEdit->toPlainText() );
    roomDescription->setIsHostRoom(true);
    roomDescription->setMaxNumberOfUsers( ui->maxUserSpinBox->value() );
    roomDescription->setAddObjectPermission( ui->addImageComboBox->currentIndex() );
    roomDescription->setRemoveObjectPermission( ui->removeImageComboBox->currentIndex() );
    roomDescription->setChangeTeamsPermission( ui->changeTeamsComboBox->currentIndex() );
    roomDescription->setHideObjectPermission( ui->hideImageComboBox->currentIndex() );
    roomDescription->setLockObjectPermission( ui->lockImageComboBox->currentIndex() );
    roomDescription->setBanPermission( ui->banUsersComboBox->currentIndex() );
    roomDescription->setTeamSortingType( ui->teamSortingComboBox->currentIndex() );
    roomDescription->setPassword( ui->passwordLineEdit->text() );
    roomDescription->setHasPassword( !ui->passwordLineEdit->text().isEmpty() );
    roomDescription->setLocation( getUserLocation() );
    roomDescription->setShowLocation(ui->locationCheckBox->isChecked());
}

void NewRoomDialog::setRoomTypeField(QString type)
{
    ui->roomTypeEdit->setText(type);
}

void NewRoomDialog::linkViewComponents(QLabel *&nicknameLb)
{
    nicknameLb = ui->nicknameLb;
}

/** Retranslates the app if its language changed: */
void NewRoomDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
    QDialog::changeEvent(event);
}

void NewRoomDialog::on_portCheckBox_toggled(bool checked)
{
    ui->portSpinBox->setEnabled(!checked);
}
