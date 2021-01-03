QT       += core gui svg network websockets gamepad
SHARE_APP_NAME = entertaining-chess

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    controllers/backgroundcontroller.cpp \
    game/MoveEngines/abstractmoveengine.cpp \
    game/MoveEngines/humanmoveengine.cpp \
    game/MoveEngines/onlinemoveengine.cpp \
    game/gameengine.cpp \
    game/gamerenderer.cpp \
    information/creditsscreen.cpp \
    main.cpp \
    mainwindow.cpp \
    online/battleplayerswidget.cpp \
    online/createprivategamescreen.cpp \
    online/joingamescreen.cpp \
    online/onlinecontroller.cpp \
    online/onlinemainmenu.cpp \
    screens/endgamescreen.cpp \
    screens/gamescreen.cpp \
    screens/mainscreen.cpp \
    screens/onlinescreen.cpp \
    screens/pausescreen.cpp \
    screens/promotescreen.cpp \
    screens/settingsscreen.cpp \
    widgets/turnbrowser.cpp

HEADERS += \
    controllers/backgroundcontroller.h \
    game/MoveEngines/abstractmoveengine.h \
    game/MoveEngines/humanmoveengine.h \
    game/MoveEngines/onlinemoveengine.h \
    game/gameengine.h \
    game/gamerenderer.h \
    information/creditsscreen.h \
    mainwindow.h \
    online/battleplayerswidget.h \
    online/createprivategamescreen.h \
    online/joingamescreen.h \
    online/onlinecontroller.h \
    online/onlinemainmenu.h \
    screens/endgamescreen.h \
    screens/gamescreen.h \
    screens/mainscreen.h \
    screens/onlinescreen.h \
    screens/pausescreen.h \
    screens/promotescreen.h \
    screens/settingsscreen.h \
    widgets/turnbrowser.h

FORMS += \
    information/creditsscreen.ui \
    mainwindow.ui \
    online/battleplayerswidget.ui \
    online/createprivategamescreen.ui \
    online/joingamescreen.ui \
    online/onlinemainmenu.ui \
    screens/endgamescreen.ui \
    screens/gamescreen.ui \
    screens/mainscreen.ui \
    screens/onlinescreen.ui \
    screens/pausescreen.ui \
    screens/promotescreen.ui \
    screens/settingsscreen.ui \
    widgets/turnbrowser.ui

QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/defaults.conf) $$shell_quote($$OUT_PWD)

unix:!macx:!android {
    # Include the-libs build tools
    include(/usr/share/the-libs/pri/buildmaster.pri)

    QT += thelib entertaining
    TARGET = entertaining-chess

    target.path = /usr/bin

    desktop.path = /usr/share/applications
    desktop.files = com.vicr123.entertaining.chess.desktop

    icon.path = /usr/share/icons/hicolor/scalable/apps/
    icon.files = assets/entertaining-chess.svg

    audio.path = /usr/share/entertaining-chess/audio
    audio.files = assets/audio/*

    defaults.files = defaults.conf
    defaults.path = /etc/entertaining-games/entertaining-chess/

    INSTALLS += target desktop icon audio defaults
}

win32 {
    # Include the-libs build tools
    include(C:/Program Files/thelibs/pri/buildmaster.pri)

    INCLUDEPATH += "C:/Program Files/thelibs/include" "C:/Program Files/libentertaining/include"
    LIBS += -L"C:/Program Files/thelibs/lib" -lthe-libs -L"C:/Program Files/libentertaining/lib" -lentertaining
}

macx {
    # Include the-libs build tools
    include(/usr/local/share/the-libs/pri/buildmaster.pri)

    QT += macextras
    LIBS += -framework CoreFoundation -framework AppKit

    blueprint {
        TARGET = "Entertaining Chess Blueprint"
#        ICON = icon-bp.icns
    } else {
        TARGET = "Entertaining Chess"
#        ICON = icon.icns
    }

    audio.files = assets/audio/
    audio.path = Contents/audio

    QMAKE_BUNDLE_DATA += audio

    INCLUDEPATH += "/usr/local/include/the-libs" "/usr/local/include/libentertaining"
    LIBS += -L/usr/local/lib -lthe-libs -lentertaining

    QMAKE_POST_LINK += $$quote(cp $${PWD}/dmgicon.icns $${PWD}/app-dmg-background.png $${OUT_PWD})
}

RESOURCES += \
    resources.qrc

DISTFILES += \
    com.vicr123.entertaining.chess.desktop \
    defaults.conf
