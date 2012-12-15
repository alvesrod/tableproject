#include "loadingview.h"
#include "ui_loadingview.h"

LoadingView::LoadingView(Room *roomView, QWidget *parent) :
    LoadingScreen(roomView, parent),
    ui(new Ui::LoadingView)
{

    ui->setupUi(this);
    ui->returnBtn->setVisible(false);
    loadingFailed = false;
}

LoadingView::~LoadingView()
{
    delete ui;
}

void LoadingView::errorWarning(QString warning)
{
    loadingFailed = true;
    ui->errorLabel->setText(warning);
    ui->returnBtn->setEnabled(true);
    ui->returnBtn->setVisible(true);
    setLoadingFailedText();
}

void LoadingView::loadingStepComplete(LoadingScreen::LoadingSteps step)
{
    if (step == LSTEP_ROOM_PREPARED) /* This is the last step before loading the room view */
        setMaxLoadingTime(MAX_LOADING_ROOM_SECONDS); //We want a different time loading the room view.
    else
        setMaxLoadingTime(MAX_LOADING_WAITING_TIME_SECONDS);

    /* Insert a dot at the end of "Loading" to give the user feedback that the loading is going forward: */
    ui->loadingLb->setText( ui->loadingLb->text() + ".");
}

/** Button pressed to return to the main menu: */
void LoadingView::on_returnBtn_clicked()
{
    QSound::play("Sound/menuClick.wav");
    emit goBackToMenu(); //Leaving the loading screen due to an error.
}

/** Warn the user that the loading failed: */
void LoadingView::setLoadingFailedText()
{
    ui->loadingLb->setText(tr("Loading Failed."));
}

/** Retranslates the app if its language changed: */
void LoadingView::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        if (loadingFailed)
            setLoadingFailedText();
    }
    QWidget::changeEvent(event);
}
