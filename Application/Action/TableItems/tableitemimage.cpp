#include "tableitemimage.h"
#include "ui_tableitemimage.h"

TableItemImage::TableItemImage(QString name, QString directory, QWidget *parent) :
    TableItem(name, parent),
    ui(new Ui::TableItemImage)
{
    /* You must set the type of the item, so that the table can know how to handle it: */
    setType(Table::ITEM_IMAGE);

    setDirectory(directory);
    ui->setupUi(this);
    ui->textLb->setTextInteractionFlags(Qt::TextBrowserInteraction);

    /* Timer to measure if the image download is having progress: */
    connect(&noProgress, SIGNAL(timeout()), this, SLOT(reponseTimeout()));
}

TableItemImage::~TableItemImage()
{
    delete ui;
}

/** Item was added to the table: */
void TableItemImage::load(bool tableLoaded)
{
    if (!tableLoaded) return; //No need to load images until the table is loaded.

    /* If the image could not be loaded, send a request: */
    if (!loadImage())
        emit tableRequest(this, Table::ITEM_REQUEST_IMAGE);
}

/** Try to load the item image: */
bool TableItemImage::loadImage()
{
    QPixmap image;
    /* Get the image if no image was loaded yet: */
    if (getItemImage().isNull()) {
        int num = searchImageFile(IMAGE_FILE_TYPE);
        if (num < 0) {
            setPreparingStatus();
            return false; //Image not found.
        }

        /* Found image, so load it from file: */
        QString filePath = makeFilepath(getName(), getDirectory(), IMAGE_FILE_TYPE, num);

        image = QPixmap(filePath);

        if ( image.isNull() ) {
            setPreparingStatus();
            return false;
        }

        setImage(image, filePath);
    }

    setReadyStatus();
    return true;
}

QString TableItemImage::getImagePath()
{
    return imagePath;
}

/* DEALING WITH PACKAGES: */

void TableItemImage::addDataToStream(QDataStream &stream)
{
    stream << text;
}

void TableItemImage::useDataFromStream(QDataStream &stream)
{
    stream >> text;
}

/* ITEM STATUS: */

void TableItemImage::onPreparingStatus()
{
    refreshSetVisible(true);
    QPixmap statusImage(":/images/imageRequest");
    ui->imageLb->setPixmap(statusImage);
    if (getOwner() == NULL)
        ui->textLb->setText( tr("Loading item..."));
    else
        ui->textLb->setText( tr("Item added by") + " <b>" + getOwner()->getNickname()
                             + "</b>.<br>" + tr("Requesting information..."));

    if (noProgress.isActive()) noProgress.stop();
    noProgress.start(SECONDS_WITHOUT_RESPONSE*1000);
    adjustSize();
}

void TableItemImage::onReceivingStatus(int progress, RoomMember *user)
{
    if ( (progress < 0) || (progress > 100) ) {
        qWarning() << "[TableItem] Error: Invalid progress: " << progress;
        return;
    }

    if (noProgress.isActive()) noProgress.stop();
    noProgress.start(SECONDS_WITHOUT_DOWNLOAD_PROGRESS*1000);

    if (getStatus() != STATUS_RECEIVING) {
        refreshSetVisible(false);
        QPixmap statusImage(":/images/imageDownload");
        ui->imageLb->setPixmap(statusImage);
        QPixmap mini;
        QDataStream stream(getFileHash());
        stream >> mini;
        if (!isHiddenForYou())
            ui->miniImgLb->setPixmap(mini);
    }

    ui->textLb->setText(tr("Downloading from") + " <b>"
                        + user->getNickname() + "</b>. "
                         +  QString::number(progress) + "%");
    adjustSize();
}

void TableItemImage::onReadyStatus()
{
    refreshSetVisible(false);
    displayOwnerToolTip();

    if (noProgress.isActive())
        noProgress.stop();

    ui->textLb->setText(text);
    displayUserPixmap();
}

void TableItemImage::onErrorStatus(QString error)
{
    refreshSetVisible(true);
    QPixmap statusImage(":/images/imageError");
    ui->imageLb->setPixmap(statusImage);
    ui->miniImgLb->setPixmap(0);
    ui->textLb->setText( "<b>" + tr("Error:") + "</b>  " + error);
    if (noProgress.isActive()) noProgress.stop();
    setToolTip(tr("Name:") + " <b>" + getName() + "</b>. ");
    adjustSize();
}

void TableItemImage::onRotatingItem(qint16 rotation)
{
    Q_UNUSED(rotation);
    if (ui->imageLb->pixmap()->isNull()) return;
    displayUserPixmap();
}

/** Add @param image to the table folder with the extension: */
bool TableItemImage::saveImageToFolder(QPixmap image, int extension)
{
    /* Create all the folders necessary if they don't exist: */
    QDir dir;
    dir.mkpath(getDirectory());

    /* Save the image into the folderpath location: */
    QString path = makeFilepath(getName(), getDirectory(), IMAGE_FILE_TYPE, extension);
    bool result = image.save(path, IMAGE_FILE_TYPE);
    if (result) {
        setImage(image, path);
    }
    return result;
}

void TableItemImage::onItemHideChanged(bool)
{
    displayUserPixmap();
}

void TableItemImage::onItemLockChanged(bool)
{
}

void TableItemImage::reponseTimeout()
{
    setErrorStatus(tr("No one seems to have information about this item."));
}

void TableItemImage::setImage(QPixmap image, QString path)
{
    qDebug() << "Image set at path " << path;
    imagePath = path;
    setItemImage(image);
    emit tableRequest(this, Table::ITEM_REQUEST_REFRESH_SHAPE);
}


bool TableItemImage::saveImage(QPixmap image)
{
    setFileHash( generateImageHash(image) );
    int extension = searchImageFile(IMAGE_FILE_TYPE);
    if (extension < 0) {
        /* File wasn't found, so create a new one: */
        extension = getAvailableFileExtension(IMAGE_FILE_TYPE);
          if (!saveImageToFolder(image, extension))
                return false;
    } else {
        /* Link to the path of the file found: */
        setImage(image, makeFilepath(getName(), getDirectory(), IMAGE_FILE_TYPE, extension));
    }
    return true;
}

bool TableItemImage::saveImage(QByteArray fileData)
{
    QString path = saveFile(fileData, IMAGE_FILE_TYPE);
    QPixmap image = QPixmap(path);
    if (image.isNull()) return false;
    setFileHash( generateImageHash(path) );
    setImage(image, path);
    return true;
}

void TableItemImage::setImageText(QString text)
{
    this->text = text;
}

QByteArray TableItemImage::generateImageHash(QString path)
{
    QPixmap map = QPixmap(path);
    return generateImageHash(map);
}

QByteArray TableItemImage::generateImageHash(QPixmap image)
{
    QByteArray data;
    QDataStream dataStream(&data, QIODevice::WriteOnly);
    dataStream << image.scaled(MINI_IMAGE_SIZE, MINI_IMAGE_SIZE);
    dataStream << image.size();
    return data;
}

/** Compare images and get how much % they match each other: */
bool TableItemImage::compareImageHash(QByteArray img1, QByteArray img2)
{
    QPixmap pix1, pix2;
    QSize size1, size2;
    QDataStream stream1(img1);
    stream1 >> pix1;
    stream1 >> size1;
    QDataStream stream2(img2);
    stream2 >> pix2;
    stream2 >> size2;
    if (size1 != size2) return false;
    int match = 0;
    QImage i1 = pix1.toImage();
    QImage i2 = pix2.toImage();
    for (int i = 0; i < i1.height(); i++) {
        for (int j = 0; j < i1.width(); j++) {
            if (i1.pixel(i,j) == i2.pixel(i,j)) match++;
        }
    }
    int totalPixels = i1.height() * i1.width();
    double porcentageMatched = ( (double) match*100 ) / ( (double) totalPixels);
    qDebug() << "Found image that matched " << porcentageMatched << "%.";
    return (porcentageMatched >= PORCENTAGE_IMAGE_MUST_MATCH);
}

int TableItemImage::searchImageFile(QString fileExt)
{
    if (getFileHash().isEmpty()) return -2;
    if (getDirectory().isEmpty()) return -3;
    int misses = 0;
    int count = 1;
    do {
        QString filepath = makeFilepath(getName(), getDirectory(), fileExt, count);
        if (QFile(filepath).exists()) {
            misses = 0;
            if (compareImageHash(getFileHash(), generateImageHash(filepath)))
                return count;
        } else misses++;
        count++;
    } while (misses < 10);
    return -1;
}

void TableItemImage::displayUserPixmap()
{
    if (getStatus() != STATUS_READY) return;

    if (isHiddenForYou()){
        ui->miniImgLb->setVisible(false);
        ui->textLb->setVisible(false);
    }

    if (getItemImage().isNull()) return;

    QTransform trans;
    trans.rotate(getRotation());
    if (getRotation() == 0)
        ui->imageLb->setPixmap(getItemImage());
    else
        ui->imageLb->setPixmap(getItemImage().transformed(trans, Qt::SmoothTransformation));
    ui->imageLb->setVisible(true);
    ui->miniImgLb->setVisible(false);
    ui->textLb->setVisible(true);

    adjustSize();
}
