#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QFontDialog>
#include <QColorDialog>

#include "../Engine/usersettings.h"

namespace Ui {
class SettingsDialog;
}

/**
 * The Settings dialog is the dialog when you press the "Settings" button.
 * It accesses the UserSettings class in the Engine (to get and set data).
 * Since this is outside the Engine, it can also access the MemberSettings
 * class that extends the UserSettings class.
 */

class SettingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    SettingsDialog(UserSettings *settings, QWidget *parent = 0);
    ~SettingsDialog();

protected:
    void changeEvent(QEvent *event);

private slots:
    void on_changeFontBtn_clicked();

    void on_changeColorBtn_clicked();

    void on_clearSettingsBtn_clicked();

    void on_okBtn_clicked();

    void on_cancelBtn_clicked();

private:
    void loadFields();
    Ui::SettingsDialog *ui;
    UserSettings *userSettings;

    void setNickname(QString name);
    void setChatColor(QColor color);
    void setChatFont(QFont font);

    QColor chatColor;
    QFont chatFont;
    QString nickname;

};

#endif // SETTINGSDIALOG_H
