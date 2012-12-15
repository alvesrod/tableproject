#ifndef ACTIONLOG_H
#define ACTIONLOG_H

#include <QWidget>
#include <QModelIndex>
#include <QListWidgetItem>


namespace Ui {
class ActionLog;
}

/**
 * @brief The ActionLog class is a component of the chatroom.
 * It displays a log dock view where it can be used outside the
 * Engine for any purpose. The chatroom is the one that calls it.
 */

class ActionLog : public QWidget
{
    Q_OBJECT
    
public:
    explicit ActionLog(QWidget *parent = 0);
    ~ActionLog();

    /** Add an item to the log message box. @return the row number: */
    void actionLog(QListWidgetItem *item);

signals:
    /** Sends a signal when the user selects a log: */
    void selected(QModelIndex index);

protected:
    /** Retranslates the app if its language changed: */
    void changeEvent(QEvent* event);
    
private slots:
    void on_logWidget_activated(const QModelIndex &index);

private:
    Ui::ActionLog *ui;
};

#endif // ACTIONLOG_H
