#include "addobjectdialog.h"
#include "ui_addobjectdialog.h"

AddObjectDialog::AddObjectDialog(RoomView *room) :
    QDialog(room, Qt::WindowSystemMenuHint | Qt::WindowTitleHint),
    ui(new Ui::AddObjectDialog)
{
    if (room == NULL) return;

    this->room = room;
    this->user = room->user();
    ui->setupUi(this);
    canHide = false;
    canLock = false;

    roomDirectory = room->info()->getRoomFolderPath();

    /* Prepare the Scene that will hold the image preview: */
    QGraphicsScene * graphicsScene = new QGraphicsScene(this);
    ui->previewGraphicsView->setScene(graphicsScene);

    ui->objectWidthSpinBox->setMinimum(MIN_IMAGE_SIZE);
    ui->objectHeightSpinBox->setMinimum(MIN_IMAGE_SIZE);
}

AddObjectDialog::~AddObjectDialog()
{
    delete ui;
}

void AddObjectDialog::showDialog()
{

    TableItemImage *item = new TableItemImage("", "");
    item->setOwner(user);

    canHide = Permission::hasObjectPermission(Permission::OBJECT_SHOW_HIDE,
                                                   room->user(), room->info(), item);

    canLock = Permission::hasObjectPermission(Permission::OBJECT_LOCK_UNLOCK,
                                                   room->user(), room->info(), item);

    ui->lockCheckBox->setEnabled(canLock);
    ui->hideCheckBox->setEnabled(canHide);

    if (!canLock)
        ui->lockCheckBox->setChecked(false);
    if (!canHide)
        ui->hideCheckBox->setChecked(false);

    item->deleteLater();

    show();
    activateWindow();
}

void AddObjectDialog::on_browseImageBtn_clicked()
{
    /* Open a file dialog for the user to search an image in his/her computer: */
    QString filePath = QFileDialog::getOpenFileName(this,tr("Open Image"), roomDirectory, tr("Image Files") + "(*.png *.jpg *.bmp *.gif)");

    if (filePath.isEmpty()) return; //User didn't type anything (probably cancelled the dialog).

    if(!QFile(filePath).exists()){
        fileNotFoundWarning(filePath);
        return;
    }

    /* Remove the folders to get only the filename from the full path: */
    QString fileName = filePath.split("/").last();
    QString name = fileName.split(".").first();

    /* Add the image name as the title of the object: */
    ui->objectNameLb->setText(name);

    /* Variable previewImage will be set here: */
    if (!getImageFromPath(filePath)) return;

    /* Set width and height spin boxes: */
    ui->objectHeightSpinBox->setValue(previewImage.size().height());
    ui->objectWidthSpinBox->setValue(previewImage.size().width());

    /* Add the image to the preview screen: */
    updatePreviewSize(previewImage.size().width(), previewImage.size().height());

    /* Enable the "Add image" button: */
    ui->addImageBtn->setEnabled(true);

    /* Focus on it for easy access: */
    ui->addImageBtn->setFocus();

}

bool AddObjectDialog::getImageFromPath(QString path)
{
    previewImage = QPixmap(path);
    if (previewImage.isNull()) {
        QMessageBox::warning(this, tr("Image fail"),
                             tr("The image could not be loaded. Please, try a different one. Location:") + " " + path);
        return false;
    }

    if (imageIsTooSmall(previewImage)) return false;

    return true;
}

/** Return true if the image is too small (and display error message to user): */
bool AddObjectDialog::imageIsTooSmall(QPixmap &image)
{
    if ( (image.size().height() < MIN_IMAGE_SIZE) || (image.size().width() < MIN_IMAGE_SIZE) ) {
        QMessageBox::warning(this, tr("Invalid image"),
                             tr("The image should be no smaller than") + " " +
                             QString::number(MIN_IMAGE_SIZE) + " " + tr("by") + " "
                             + QString::number(10) + " " + tr("pixels."));
        return true;
    }
    return false;
}

/** Called when the value in the spin box changed to change image size: */
void AddObjectDialog::on_objectWidthSpinBox_valueChanged(int width)
{
    if (previewImage.isNull()) return;
    updatePreviewSize(width, ui->objectHeightSpinBox->value());
}

/** Called when the value in the spin box changed to change image size: */
void AddObjectDialog::on_objectHeightSpinBox_valueChanged(int height)
{
    if (previewImage.isNull()) return;
    updatePreviewSize(ui->objectWidthSpinBox->value(), height);
}

void AddObjectDialog::updatePreviewSize(int width, int height)
{
    if (previewImage.isNull()) return;
    previewImageScaled = previewImage.scaled(QSize(width, height));
    ui->previewGraphicsView->scene()->clear();
    ui->previewGraphicsView->scene()->addPixmap( previewImageScaled );
    ui->previewGraphicsView->scene()->setSceneRect(previewImageScaled.rect());
}

void AddObjectDialog::on_addImageBtn_clicked()
{

    QString name = ui->objectNameLb->text();
    if (name.isEmpty()) {
        ui->objectNameLb->setText(UNNAMMED_IMAGE);
        return;
    }

    TableItemImage *item = new TableItemImage(name, roomDirectory);
    QList<qint8> allTeams = getArrayWithAllTeams();
    if ( (ui->hideCheckBox->isChecked()) && (canHide) )
        item->setHiddenTeams(allTeams);
    if ( (ui->lockCheckBox->isChecked()) && (user != NULL) && (canLock) ) {
        allTeams.removeOne(user->getTeam());
        item->setLockedTeams(allTeams);
    }
    if (!item->saveImage(previewImageScaled))
        item->deleteLater();
    else {
        item->setImageText( ui->textEdit->toPlainText().left(MAX_TEXT_LENGTH) );
        emit sendActionAddObject(item); /* Alert everyone about the new image item: */
    }
    close();
}

void AddObjectDialog::fileNotFoundWarning(QString location)
{
    QMessageBox::warning(this, tr("File not found"),
                         tr("The file could not be found at location:") + " " + location);
}


QList<qint8> AddObjectDialog::getArrayWithAllTeams()
{
    QList<qint8> teams;
    int size = RoomMember::getTeamList().size();
    for (int i = 0; i < size; i++) teams << i;
    return teams;
}

/** Retranslates the app if its language changed: */
void AddObjectDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
    QDialog::changeEvent(event);
}

void AddObjectDialog::on_addAllBtn_clicked()
{
    RoomInfo *roomToLoad = new RoomInfo(0,"",this);
    QString path = qApp->applicationDirPath() + "/" + ROOM_FOLDER_NAME;
    QString filePath = QFileDialog::getOpenFileName(this,tr("Get Table Items"), path,
                                                    tr("Table Files ") + "(*." + TABLE_FILE_TYPE + ")");
    if (filePath.isEmpty()) {
        roomToLoad->deleteLater();
        return;
    }

    if (! roomToLoad->loadRoomFile(filePath) ) {
        QMessageBox::information(this, tr("Invalid File"), tr("The file loaded is invalid. Please, try a different one."));
        roomToLoad->deleteLater();
        return;
    }

    Table *tableToLoad = roomToLoad->getTable();
    if (tableToLoad == NULL) return;

    /*
     * The necessity of putting items in a list and then sorting this
     * list based on the z-position is because every item added to the
     * table is brought to the front of all items. This souldn't happen
     * in this particular case because we already have a defined order
     * from the other table. So, we have to sort it.
     */
    QMap<qreal, TableItem*> items;

    foreach (TableItemDecorator *itemToLoad, tableToLoad->getTableItems()) {

        /* Add only image itens since this dialog is about image items: */
        TableItemImage *item = dynamic_cast<TableItemImage*>(itemToLoad->getItem());
        if (item == NULL) continue;

        /* Make sure the image file actually exists: */
        if (!getImageFromPath(filePath.left(filePath.lastIndexOf("/")) + "/" + item->getName()
                              + "." + IMAGE_FILE_TYPE)) continue;

        /* Change the directory because now this item belongs to a different table: */
        item->setDirectory(roomDirectory);

        if (item->saveImage(previewImage)) { //previewImage was set in "getImageFromPath()"
            items[item->getZPosition()] = item;
        }
    }

    /* Alert the user if not all items were added: */
    if (items.size() < tableToLoad->getTableItems().size())
        QMessageBox::information(this, tr("Not all items were added"),
                             tr("Not all items were added (only") + " " + QString::number(items.size()) + " "
                                + tr("out of") + " " + QString::number(tableToLoad->getTableItems().size()) + tr(")."));

    /* QMap is sorted, so you will send the list of items sorted: */
    foreach(TableItem* item, items.values())
        emit sendActionAddObject(item); /* Alert everyone about the new image item: */


    tableToLoad->deleteLater(); //All items will be deleted here since they belong to the table
    roomToLoad->deleteLater();
    close();
}

void AddObjectDialog::on_textEdit_textChanged()
{
    if (ui->textEdit->toPlainText().length() > MAX_TEXT_LENGTH)
        ui->textEdit->textCursor().deletePreviousChar();
}
