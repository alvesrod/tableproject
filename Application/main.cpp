#include <QtGui/QApplication>
#include <QTranslator>

#include "../Engine/engine.h"
#include "EngineImpl/enginefactory.h"
#include "constants.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    /* Set the Application name. The Engine can make use of that: */
    a.setApplicationName(APP_NAME);

    /* Set the Application company. The Engine can make use of that: */
    a.setOrganizationName(APP_COMPANY); 

    /* Declare a new engine: */
    Engine *engine = new Engine( new EngineFactory(), APP_VERSION, APP_SECRET, ENG_VERSION, argc, argv);

    /* Start the engine. The engine will take care of displaying the GUI: */
    engine->start();

    return a.exec();
}
