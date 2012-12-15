#ifndef MAINBUTTON_H
#define MAINBUTTON_H

#include <QLabel>
#include <QDebug>
#include <QSound>

/**
  This is the class for the Label buttons in the main menu.
 */

class MainButton : public QLabel
{
    Q_OBJECT
public:
    explicit MainButton(QWidget *parent = 0);

protected:
    void enterEvent ( QEvent *);
    void leaveEvent ( QEvent *);
    void mousePressEvent ( QMouseEvent *);
    
signals:
    void labelClicked();
    
public slots:

private:
    QString originalStyle;
    QString mouseOverStyle;
    
};

#endif // MAINBUTTON_H
