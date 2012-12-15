#-------------------------------------------------
#
# Project created by QtCreator 2012-10-21T20:28:32
#
#-------------------------------------------------

QT       += core gui network

TARGET = Project-Live-Table
TEMPLATE = app
RC_FILE = appIcon.rc

#All the idioms besides English that the app translates:
TRANSLATIONS = Translations/apptr_pt.ts \
               Translations/apptr_zh.ts \
#You can add more here. The .ts files will be created after you update the translation.
#You should also add them below in the "OTHER FILES" and the .qm ones to the Resources file.

SOURCES += main.cpp\
    EngineImpl/mainwindow.cpp \
    Widgets/mainbutton.cpp \
    Action/actioncontroller.cpp \
    EngineImpl/Views/loadingview.cpp \
    EngineImpl/Views/mainmenu.cpp \
    EngineImpl/Views/roomview.cpp \
    EngineImpl/Views/findroomdialog.cpp \
    EngineImpl/Views/newroomdialog.cpp \
    EngineImpl/Views/userdetails.cpp \
    Views/settingsdialog.cpp \
    EngineImpl/roommember.cpp \
    EngineImpl/membersettings.cpp \
    EngineImpl/roominfo.cpp \
    EngineImpl/enginefactory.cpp \
    Action/Views/throwdice.cpp \
    Action/Views/addobjectdialog.cpp \
    Action/tableitem.cpp \
    Action/Views/table.cpp \
    Action/dialogcontroller.cpp \
    Action/permission.cpp \
    Action/tablecontroller.cpp \
    Action/TableItems/tableitemimage.cpp \
    Action/Views/lockhidedialog.cpp \
    Action/Views/roominfodialog.cpp \
    Action/tableitemdecorator.cpp

HEADERS  += \
    EngineImpl/mainwindow.h \
    EngineImpl/Views/mainmenu.h \
    EngineImpl/Views/loadingview.h \
    EngineImpl/Views/roomview.h \
    EngineImpl/Views/findroomdialog.h \
    EngineImpl/Views/newroomdialog.h \
    EngineImpl/Views/userdetails.h \
    Views/settingsdialog.h \
    Widgets/mainbutton.h \
    Action/actioncontroller.h \
    EngineImpl/roommember.h \
    EngineImpl/membersettings.h \
    EngineImpl/roominfo.h \
    EngineImpl/enginefactory.h \
    constants.h \
    Action/Views/throwdice.h \
    Action/Views/addobjectdialog.h \
    Action/tableitem.h \
    Action/Views/table.h \
    Action/dialogcontroller.h \
    Action/permission.h \
    Action/tablecontroller.h \
    Action/TableItems/tableitemimage.h \
    Action/Views/lockhidedialog.h \
    Action/Views/roominfodialog.h \
    Action/tableitemdecorator.h


FORMS    += EngineImpl/mainwindow.ui \
    EngineImpl/Views/mainmenu.ui \
    EngineImpl/Views/loadingview.ui \
    EngineImpl/Views/roomview.ui \
    EngineImpl/Views/findroomdialog.ui \
    EngineImpl/Views/newroomdialog.ui \
    EngineImpl/Views/userdetails.ui \
    Views/settingsdialog.ui \
    Action/Views/throwdice.ui \
    Action/Views/addobjectdialog.ui \
    Action/Views/table.ui \
    Action/TableItems/tableitemimage.ui \
    Action/Views/lockhidedialog.ui \
    Action/Views/roominfodialog.ui

RESOURCES += \
    Resources.qrc

OTHER_FILES += \
    Translations/apptr_pt.ts \
    Translations/apptr_zh.ts \

# add the engine library to the application:
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Engine/release/ -lEngine
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Engine/debug/ -lEngine
else:symbian: LIBS += -lEngine
else:unix: LIBS += -L$$OUT_PWD/../Engine/ -lEngine

INCLUDEPATH += $$PWD/../Engine
DEPENDPATH += $$PWD/../Engine
