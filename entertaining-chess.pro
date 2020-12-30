QT       += core gui entertaining svg network websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    game/MoveEngines/abstractmoveengine.cpp \
    game/MoveEngines/humanmoveengine.cpp \
    game/MoveEngines/onlinemoveengine.cpp \
    game/gameengine.cpp \
    game/gamerenderer.cpp \
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
    widgets/turnbrowser.cpp

HEADERS += \
    game/MoveEngines/abstractmoveengine.h \
    game/MoveEngines/humanmoveengine.h \
    game/MoveEngines/onlinemoveengine.h \
    game/gameengine.h \
    game/gamerenderer.h \
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
    widgets/turnbrowser.h

FORMS += \
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
    widgets/turnbrowser.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
