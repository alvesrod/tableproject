#ifndef MAINMENU_H
#define MAINMENU_H

#include <QWidget>
#include <QDebug>
#include "constants.h"
#include "../Engine/menu.h"

namespace Ui {
class MainMenu;
}

#define NEWS_TITLE_STYLE "<span style='font-size:8pt; color:#7f7f7f;'><b>"

/**
 * This is the Main Menu that is displayed inside the app window. The main
 * menu allows you to open the NewRoom and FindRoom dialogs. If you are
 * unsure about which class is this, check its Form in the Forms folder.
 */

class MainMenu : public Menu
{
    Q_OBJECT
    
public:
    MainMenu(UserSettings *settings, QWidget *parent = 0);
    ~MainMenu();

protected:
    /**
     * This class has to implement this method to tell the Engine where
     * the web server is located:
     */
    QString getWebServerURL();

    /**
     * Implements the engine method to decide how to compose the news
     * based on its date and text:
     */
    QString getNews(QString date, QString text);

    /**
     * Implements the engine method to decide where to display
     * the news. @param news is actually the return value of
     * the getNews() method.
     */
    void setNews(QString news);

    /** Retranslates the app if its language changed: */
    void changeEvent(QEvent *event);
    
private:
    void setWebsiteURL();
    void setAppVersion();
    Ui::MainMenu *ui;

};

#endif // MAINMENU_H
