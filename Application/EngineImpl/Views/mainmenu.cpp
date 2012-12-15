#include "mainmenu.h"
#include "ui_mainmenu.h"

MainMenu::MainMenu(UserSettings *settings, QWidget *parent) :
    Menu(settings, parent),
    ui(new Ui::MainMenu)
{
    ui->setupUi(this);

    /* Connections for the 3 main buttons (New Room, Find Room, and Exit): */
    connect(ui->newRoomLabel, SIGNAL(labelClicked()), this, SLOT(openNewRoomDialog()));
    connect(ui->findRoomLabel, SIGNAL(labelClicked()), this, SLOT(openFindRoomDialog()));
    connect(ui->exitLabel, SIGNAL(labelClicked()), this, SLOT(closeApp()));

    /* Display a link to the website in the right corner: */
    setWebsiteURL();

    /* Display the version of the application in the left corner: */
    setAppVersion();

    /* Ask the web server for the news. When it arrives, getNews() will be called: */
    requestNews();
}

MainMenu::~MainMenu()
{
    delete ui;
}

QString MainMenu::getWebServerURL()
{
    return SERVERS_CONTACT_ADDR;
}

QString MainMenu::getNews(QString date, QString text)
{
    return QString(NEWS_TITLE_STYLE) + "News from " + date + ":</b><br>" + text.remove(0,3);
    //remove(0,3) removes the first '<p>' from the news (to put them in the same line).
}

void MainMenu::setNews(QString news)
{
    ui->latestNews->setText(news);
}

void MainMenu::setWebsiteURL()
{
    //The label can be clicked to go to the website.
    ui->websiteLb->setText(QString("<a href='") + SERVERS_CONTACT_ADDR
                           + "' style='text-decoration: none'><font color='#727A83'> "
                           + tr("Visit Website") + "</font></a>");
    ui->websiteLb->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->websiteLb->setToolTip(tr("Go to") + " " + SERVERS_CONTACT_ADDR);
}

void MainMenu::setAppVersion()
{
    ui->versionLb->setText(tr("Version") + " " + Engine::getAppVersion()
                               + " (" + tr("using engine") + " "
                               + Engine::getEngineVersion() + "). ");
}

/** Retranslates the app if its language changed: */
void MainMenu::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        setNews(getOldNews());
        setWebsiteURL();
        setAppVersion();
    }
    QWidget::changeEvent(event);
}
