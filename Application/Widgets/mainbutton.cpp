#include "mainbutton.h"

MainButton::MainButton(QWidget *parent) :
    QLabel(parent)
{
    /* Change color to red when the mouse is over the label: */
    mouseOverStyle = QString::fromUtf8("color:rgb(180, 70, 70)");
}

/** When the mouse is over the label: */
void MainButton::enterEvent(QEvent *)
{
    if ( originalStyle.isEmpty() )
        originalStyle = styleSheet();
    setStyleSheet(mouseOverStyle);
}

/** When the mouse leaves the label: */
void MainButton::leaveEvent(QEvent *)
{
    setStyleSheet(originalStyle);
}

void MainButton::mousePressEvent(QMouseEvent *)
{
    QSound::play("Sound/menuClick.wav");
    emit labelClicked();
}
