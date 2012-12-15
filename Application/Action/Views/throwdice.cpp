#include "throwdice.h"
#include "ui_throwdice.h"

ThrowDice::ThrowDice(QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint),
    ui(new Ui::ThrowDice)
{
    ui->setupUi(this);
    welcomeLabel();
}

ThrowDice::~ThrowDice()
{
    delete ui;
}

void ThrowDice::welcomeLabel()
{
    ui->infoLabel->setText(tr("Set the boundary below."));
    ui->infoLabel->setStyleSheet("color: rgb(70, 126, 40);"); //Green
}

void ThrowDice::on_throwDiceBtn_clicked()
{
    if (ui->maxSpinBox->value() <= ui->minSpinBox->value()) {
        ui->infoLabel->setText(tr("Invalid boundary."));
        ui->infoLabel->setStyleSheet("color: rgb(220, 2, 2)"); //Red
        return;
    }

    emit throwDice( ui->minSpinBox->value(), ui->maxSpinBox->value(), ui->countSpinBox->value() );
    welcomeLabel(); //Restore the label since the dialog is about to close
    close();
}

void ThrowDice::on_minSpinBox_valueChanged(int arg1)
{
    /* Make sure the max boundary is bigger than the min boundary: */
    if (ui->maxSpinBox->value() <= arg1)
        ui->maxSpinBox->setValue(arg1 + 1);
}

void ThrowDice::on_maxSpinBox_valueChanged(int arg1)
{
    /* Make sure the min boundary is smaller than the max boundary: */
    if (ui->minSpinBox->value() >= arg1)
        ui->minSpinBox->setValue(arg1 - 1);
}

/** Retranslates the app if its language changed: */
void ThrowDice::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
    QDialog::changeEvent(event);
}
