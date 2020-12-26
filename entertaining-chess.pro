QT       += core gui entertaining svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    game/MoveEngines/abstractmoveengine.cpp \
    game/MoveEngines/humanmoveengine.cpp \
    game/gameengine.cpp \
    game/gamerenderer.cpp \
    main.cpp \
    mainwindow.cpp \
    screens/gamescreen.cpp \
    screens/mainscreen.cpp

HEADERS += \
    game/MoveEngines/abstractmoveengine.h \
    game/MoveEngines/humanmoveengine.h \
    game/gameengine.h \
    game/gamerenderer.h \
    mainwindow.h \
    screens/gamescreen.h \
    screens/mainscreen.h

FORMS += \
    mainwindow.ui \
    screens/gamescreen.ui \
    screens/mainscreen.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
