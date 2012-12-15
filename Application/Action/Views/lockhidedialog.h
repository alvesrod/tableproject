#ifndef LOCKHIDEDIALOG_H
#define LOCKHIDEDIALOG_H

#include <QDialog>
#include "Action/tableitem.h"
#include "EngineImpl/Views/roomview.h"

namespace Ui {
class LockHideDialog;
}

/**
 * @brief The LockHideDialog class is the dialog
 * that shows up when the user wants to lock or hide
 * a table item. This dialog displays the list of teams,
 * so the user can check them.
 */

class LockHideDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit LockHideDialog(RoomView *room = 0);
    ~LockHideDialog();

    void openLockDialog(TableItem *item);
    void openHideDialog(TableItem *item);

signals:
    void lockItems(TableItem *item, QList<qint8> teams);
    void hideItems(TableItem *item, QList<qint8> teams);

protected:
    void changeEvent(QEvent *event);
    
private slots:
    void on_OkButtonBox_accepted();

    void on_OkButtonBox_rejected();

    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_genericBtn1_clicked();

    void on_genericBtn2_clicked();

private:
    void loadDialog();
    void makeStringListOfTeams();
    void makeListOfTeams();
    void setHideDialog();
    void setLockDialog();
    void updateScreenText();
    void makeListOfOtherTeams();
    void sendAndClose();
    Ui::LockHideDialog *ui;
    bool isLockDialog;
    QList<qint8> teams;
    TableItem *item;
    QStringList teamString;
    RoomView *room;
};

#endif // LOCKHIDEDIALOG_H
