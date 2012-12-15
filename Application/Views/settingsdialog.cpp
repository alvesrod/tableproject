#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(UserSettings *settings, QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint),
    ui(new Ui::SettingsDialog)
{

    if (settings == NULL) {
        qWarning() << "Cannot initialize SettingsDialog without User Settings.";
        return;
    }

    userSettings = settings;
    ui->setupUi(this);

    /*
     * Set the background color to white:
     */
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setPalette(pal);

    loadFields();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_changeFontBtn_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, QFont(ui->messageFontLb->text()), this);
    if (ok)
        setChatFont(font);
}

void SettingsDialog::on_changeColorBtn_clicked()
{
    setChatColor ( QColorDialog::getColor(Qt::black, this) );
}

void SettingsDialog::on_clearSettingsBtn_clicked() //Restore defaults
{
    setChatColor( userSettings->getDefaultChatColor() );
    setChatFont( userSettings->getDefaultFont() );
    setNickname( userSettings->getDefaultNickname() );
}

void SettingsDialog::loadFields()
{
    setChatColor( userSettings->getChatColor() );
    setChatFont( userSettings->getChatFont() );
    setNickname( userSettings->getNickname() );
}

void SettingsDialog::setNickname(QString name)
{
    nickname = name;
    ui->nicknameLineEdit->setText( nickname );
}

void SettingsDialog::setChatColor(QColor color)
{
    if (!color.isValid()) {
        qDebug() << "[Settings Dialog] Invalid color.";
        return;
    }
    chatColor = color;
    ui->messageColorLb->setPalette( QPalette(chatColor) );
}

void SettingsDialog::setChatFont(QFont font)
{
    chatFont = font;
    ui->messageFontLb->setText(chatFont.family());
    ui->messageFontLb->setFont(chatFont);

}

/** Retranslates the app if its language changed: */
void SettingsDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
    QDialog::changeEvent(event);
}

void SettingsDialog::on_okBtn_clicked()
{
    if (ui->nicknameLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Name"), tr("You need to type a name first."));
        return;
    }

    setNickname( ui->nicknameLineEdit->text() );

    if (nickname != userSettings->getNickname())
       userSettings->setNickname( nickname, SEND_SIGNAL_AND_SAVE );

    if (chatColor != userSettings->getChatColor())
       userSettings->setChatColor( chatColor, SEND_SIGNAL_AND_SAVE );

    if (chatFont != userSettings->getChatFont())
       userSettings->setChatFont( chatFont, SEND_SIGNAL_AND_SAVE );

    close();
}

void SettingsDialog::on_cancelBtn_clicked()
{
    close();
}
