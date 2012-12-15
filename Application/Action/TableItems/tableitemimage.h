#ifndef TABLEITEMIMAGE_H
#define TABLEITEMIMAGE_H

#include <QWidget>
#include <QObject>
#include "Action/tableitem.h"
#include "Action/Views/table.h"
#include <QBitmap>

class TableItem;

namespace Ui {
class TableItemImage;
}

#define PORCENTAGE_IMAGE_MUST_MATCH 84 //How much the images must match to be considered the same (for hash).
#define MINI_IMAGE_SIZE 32 //Size of the mini image used for comparison
#define SECONDS_WITHOUT_RESPONSE 15 //More than this seconds without response, the image has an error.
#define SECONDS_WITHOUT_DOWNLOAD_PROGRESS 30 //error if the download stops for a while

/**
 * @brief The TableItemImage class extends TableItem to define
 * a more specific type of item (an item that holds images).
 */

class TableItemImage : public TableItem
{
    Q_OBJECT
    
public:
    /**
     * @param name: the name of the image. This name must match the name of the file (without extension).
     * @param directory: where the file is located.
     */
    TableItemImage(QString name, QString directory, QWidget *parent = 0);
    ~TableItemImage();

    /**
     * Called when the item was added to the table (or during loading screen).
     * This implements load() in the TableItem class (check it out).
     * Ps: This can also be called for some other reason, such as ask again
     * for an image.
     */
    void load(bool tableLoaded = true);

    /**
     * Asks the item to load the image from the file in the local directory.
     * Requests an image to others in the network if the file was not found.
     * @return true if the image was loaded.
     */
    bool loadImage();

    /** Get the image file path (empty if image wasn't loaded): */
    QString getImagePath();

    /** Save to the Table folder an image, but make sure there are no overwrites. */
    bool saveImage(QPixmap image);

    /** Exactly like the method below, but this one saves based on a network package: */
    bool saveImage(QByteArray fileData);

    /** Set the text that display together with an image: */
    void setImageText(QString text);

    /* ITEM STATUS */
    void onPreparingStatus(); //Item is waiting to be shown.
    void onReceivingStatus(int progress, RoomMember *user); //Item is receiving a file to be shown.
    void onReadyStatus(); //Item is being displayed.
    void onErrorStatus(QString error); //Item cannot be displayed because of something.

protected:

    /* Implements the superclass methods to deal with messages: */
    void addDataToStream(QDataStream &stream);
    void useDataFromStream(QDataStream &stream);

    void onItemHideChanged(bool hidden); //called when the item was hidden or shown to you;
    void onItemLockChanged(bool); //called when the item was locked/unlocked for you;

    /* ROTATION */
    void onRotatingItem(qint16 rotation);

private slots:
    /** Called when the item is waiting response from the network for a way too long. Display error message. */
    void reponseTimeout();
    
private:
    void setImage(QPixmap image, QString path);

    /** Compare if 2 image hashes match. They don't have to match perfectly: */
    bool compareImageHash(QByteArray img1, QByteArray img2);

    /** Save to the Table folder an image (helper method): */
    bool saveImageToFolder(QPixmap image, int extension);

    /** Static method to get a hash of an image at @param path. This is expensive. */
    static QByteArray generateImageHash(QString path);
    static QByteArray generateImageHash(QPixmap image);

    /**
     * @brief search for an image locally based on the name of the item and table.
     * This is a very expensive method.
     * @param fileExt: the extension of the file (eg: .txt).
     * @return -1 if file not found. -2 you don't have a hash. Otherwise, it
     * return the extension number of the file (1 if no extension).
     */
    int searchImageFile(QString fileExt);

    /** Display the user image: */
    void displayUserPixmap();

    /* The file path for the image currently being displayed: */
    QString imagePath;

    /* Timer to measure if the image is stucked at a downloading screen or receiving: */
    QTimer noProgress;

    Ui::TableItemImage *ui;

    QString text;
};

#endif // TABLEITEMIMAGE_H
