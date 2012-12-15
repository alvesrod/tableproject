#-------------------------------------------------
#
# Project created by QtCreator 2012-10-21T18:11:50
#
#-------------------------------------------------

QT       += network

TARGET = Engine
TEMPLATE = lib

#All the idioms besides English that the app translates:
TRANSLATIONS = Translations/enginetr_pt.ts \
               Translations/enginetr_zh.ts \
#You can add more here. The .ts files will be created after you update the translation.
#You should also add them below in the "OTHER FILES" and the .qm ones to the Resources file.

DEFINES += ENGINE_LIBRARY

SOURCES += \
    webcontact.cpp \
    Views/chatroom.cpp \
    Views/privatechatbox.cpp \
    Views/userbuttonbox.cpp \
    viewcontroller.cpp \
    usersettings.cpp \
    user.cpp \
    update.cpp \
    roomdescription.cpp \
    roomcontroller.cpp \
    pingroomtask.cpp \
    loadingscreen.cpp \
    engine.cpp \
    Network/tcpserver.cpp \
    Network/tcpclient.cpp \
    Network/network.cpp \
    Views/downloader.cpp \
    room.cpp \
    findroom.cpp \
    newroom.cpp \
    menu.cpp \
    userbox.cpp \
    filetransfer.cpp \
    moveflow.cpp \
    Views/actionlog.cpp \
    Views/updater.cpp


HEADERS +=\
        Engine_global.h \
    webcontact.h \
    viewcontroller.h \
    usersettings.h \
    user.h \
    update.h \
    roomdescription.h \
    roomcontroller.h \
    pingroomtask.h \
    packagehandlerinterface.h \
    loadingscreen.h \
    enginefactoryinterface.h \
    Engine_global.h \
    engine.h \
    Views/chatroom.h \
    Network/tcpserver.h \
    Network/tcpclient.h \
    Network/network.h \
    Views/privatechatbox.h \
    Views/downloader.h \
    room.h \
    findroom.h \
    newroom.h \
    menu.h \
    Views/userbuttonbox.h \
    userbox.h \
    filetransfer.h \
    moveflow.h \
    Views/actionlog.h \
    Views/updater.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xEB78F5FB
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = Engine.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

FORMS += \
    Views/privatechatbox.ui \
    Views/userbuttonbox.ui \
    Views/chatroom.ui \
    Views/downloader.ui \
    Views/actionlog.ui \
    Views/updater.ui

OTHER_FILES += \
    Translations/enginetr_pt.ts \
    Translations/enginetr_zh.ts \

RESOURCES += \
    EResources.qrc
