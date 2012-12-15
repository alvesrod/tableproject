#ifndef THROWDICE_H
#define THROWDICE_H

#include <QDialog>

namespace Ui {
class ThrowDice;
}


/**
 * @brief The ThrowDice class is a dialog that allows the user to
 * throw a dice. The user will choose the min. and max. values of
 * the dice, and a throw dice function in the RoomView will handle
 * that.
 */

class ThrowDice : public QDialog
{
    Q_OBJECT
    
public:
    ThrowDice(QWidget *parent = 0);
    ~ThrowDice();

signals:
    /** Send the result of the dice to whatever catches it: */
    void throwDice(qint32 min, qint32 max, qint32 count = 1);

protected:
    void changeEvent(QEvent *event);
    
private slots:
    /** When the throw dice button is clicked: */
    void on_throwDiceBtn_clicked();

    void on_minSpinBox_valueChanged(int arg1);

    void on_maxSpinBox_valueChanged(int arg1);

private:
    /** Label that is displayed when the window is opened: */
    void welcomeLabel();

    Ui::ThrowDice *ui;
};

#endif // THROWDICE_H
