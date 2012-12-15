#ifndef LOADINGVIEW_H
#define LOADINGVIEW_H

#include <QWidget>
#include "mainmenu.h"
#include "constants.h"
#include "../Engine/loadingscreen.h"
#include "EngineImpl/Views/roomview.h"

namespace Ui {
class LoadingView;
}

/**
  * This is the view for the loading screen. It implements the Engine
  * class LoadingScreen.
  */

class LoadingView : public LoadingScreen
{
    Q_OBJECT
    
public:
    explicit LoadingView(Room *roomView, QWidget *parent = 0);
    ~LoadingView();
    
protected slots:
    /** Called when an error ocurred. The loading was cancelled: */
    void errorWarning(QString warning);

    /**
     * The loading screen sends steps that can be used to give to
     * the user some feedback of how far is it from loading the room:
     */
    void loadingStepComplete(LoadingSteps step);

    /** Retranslates the app if its language changed: */
    void changeEvent(QEvent *event);

private slots:
    void on_returnBtn_clicked();

private:
    void setLoadingFailedText();
    Ui::LoadingView *ui;
    bool loadingFailed;
};

#endif // LOADINGVIEW_H
