#ifndef ADDOBJECTDIALOG_H
#define ADDOBJECTDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QDebug>
#include <QDir>
#include <QtAlgorithms>
#include <QMap>
#include <QHash>

#include "Action/TableItems/tableitemimage.h"
#include "constants.h"
#include "userbox.h"
#include "Action/permission.h"
#include "EngineImpl/roommember.h"
#include "Action/actioncontroller.h"

#define UNNAMMED_IMAGE "Unnamed"
#define MIN_IMAGE_SIZE 10 //pixels
#define MAX_TEXT_LENGTH 800 //characters

namespace Ui {
class AddObjectDialog;
}

/**
 * @brief The AddObjectDialog class is a dialog that allows the user
 * to choose an object to be added to the table (such as an image).
 */

class AddObjectDialog : public QDialog
{
    Q_OBJECT
    
public:
    AddObjectDialog(RoomView *room);
    ~AddObjectDialog();

    /* Display the dialog: */
    void showDialog();

signals:
    /** Send a message saying that there's a new object in the table: */
    void sendActionAddObject(TableItem *item);

protected:
    void changeEvent(QEvent *event);
    
private slots:

    void on_browseImageBtn_clicked();

    void on_objectWidthSpinBox_valueChanged(int width);

    void on_objectHeightSpinBox_valueChanged(int height);

    void on_addImageBtn_clicked();

    void on_addAllBtn_clicked();

    void on_textEdit_textChanged();

private:
    void updatePreviewSize(int width, int height);
    int getExtension(QString name, QString folderpath, QString imageFilePath, QPixmap image);
    bool compareImages(QString path1, QString path2, QPixmap image);
    void fileNotFoundWarning(QString location);
    bool imageIsTooSmall(QPixmap &image);
    bool getImageFromPath(QString path);
    static QList<qint8> getArrayWithAllTeams();

    QPixmap previewImageScaled;
    QPixmap previewImage;
    QString roomDirectory;
    RoomMember *user;
    RoomView *room;
    Ui::AddObjectDialog *ui;

    bool canLock;
    bool canHide;
};

#endif // ADDOBJECTDIALOG_H
