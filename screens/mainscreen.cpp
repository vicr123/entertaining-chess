/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "mainscreen.h"
#include "ui_mainscreen.h"

#include "controllers/backgroundcontroller.h"
#include "game/gameengine.h"
#include "game/MoveEngines/humanmoveengine.h"
#include <musicengine.h>
#include <loadoverlay.h>
#include <questionoverlay.h>
#include <the-libs_global.h>
#include <online/logindialog.h>
#include <online/onlineapi.h>
#include "online/onlinecontroller.h"
#include <QPainter>
#include <QSvgRenderer>
#include <tvariantanimation.h>
#include <gamepadevent.h>
#include <QGraphicsOpacityEffect>
#include <QMouseEvent>
#include <QKeyEvent>
#include <gamepadbuttons.h>
#include "information/creditsscreen.h"

struct MainScreenPrivate {
    tVariantAnimation* hudOpacity;
    bool splashHidden = false;
};

MainScreen::MainScreen(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::MainScreen) {
    ui->setupUi(this);
    d = new MainScreenPrivate();

//    ui->topSpacer->changeSize(0, SC_DPI(50), QSizePolicy::Preferred, QSizePolicy::Fixed);
//    ui->leftSpacer->changeSize(SC_DPI(50), 0, QSizePolicy::Fixed, QSizePolicy::Preferred);
    ui->spacer1->changeSize(0, SC_DPI(50), QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->topBarrier->setBounceWidget(ui->playButton);
    ui->bottomBarrier->setBounceWidget(ui->exitButton);
    ui->exitButton->setProperty("type", "destructive");

    ui->splashActionLabel->setText(tr("%1 Start").arg(GamepadButtons::stringForKey(QKeySequence(Qt::Key_Enter)).append(" ").append(GamepadButtons::stringForButton(QGamepadManager::ButtonA))));

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Exit"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [ = ] {
        ui->exitButton->click();
    });

    QGraphicsOpacityEffect* gamepadHudOpacity = new QGraphicsOpacityEffect();
    ui->gamepadHud->setGraphicsEffect(gamepadHudOpacity);
    gamepadHudOpacity->setOpacity(0);

    d->hudOpacity = new tVariantAnimation(this);
    d->hudOpacity->setStartValue(0.0);
    d->hudOpacity->setEndValue(1.0);
    d->hudOpacity->setDuration(250);
    d->hudOpacity->setEasingCurve(QEasingCurve::OutCubic);
    connect(d->hudOpacity, &tVariantAnimation::valueChanged, this, [ = ](QVariant value) {
        gamepadHudOpacity->setOpacity(value.toReal());
    });

    this->setFocusProxy(ui->stackedWidget);
    ui->menuPage->setFocusProxy(ui->playButton);

    BackgroundController::instance()->install(this);
}

MainScreen::~MainScreen() {
    delete d;
    delete ui;
}

void MainScreen::on_exitButton_clicked() {
    QApplication::exit();
}

void MainScreen::on_loadButton_clicked() {
    MusicEngine::playSoundEffect(MusicEngine::Selection);

    LoadOverlay* load = new LoadOverlay(this);
    connect(load, &LoadOverlay::loadData, this, [ = ](QDataStream * stream) {
        GameEnginePtr engine(new GameEngine(new HumanMoveEngine(), new HumanMoveEngine()));
        if (!engine->loadGame(stream)) {
            QuestionOverlay* question = new QuestionOverlay(this);
            question->setStandardDialog(QuestionOverlay::FileCorrupt);
            connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
            connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
        } else {
            this->animateOut([ = ] {
                emit startGame(engine);
            });
        }
    });
    load->load();
}

void MainScreen::on_playButton_clicked() {
    GameEnginePtr engine(new GameEngine(new HumanMoveEngine(), new HumanMoveEngine()));
    engine->startGame();
    this->animateOut([ = ] {
        emit startGame(engine);
    });
}

void MainScreen::on_playOnlineButton_clicked() {
    LoginDialog* login = new LoginDialog(this);
    if (login->exec()) {
//        ui->stackedWidget->setCurrentWidget(ui->onlineScreen);
//        ui->onlineScreen->connectToOnline();

//        ReportController::setAutomaticReportingEnabled(this, true);
        this->animateOut([ = ] {
            OnlineController::instance()->connectToOnline();
        });
    } else {

    }
}

void MainScreen::animateOut(std::function<void ()> after) {
//    QMetaObject::Connection* c = new QMetaObject::Connection;
//    *c = connect(d->backgroundOffsetTop, &tVariantAnimation::finished, this, [ = ] {
//        after();
//        disconnect(*c);
//        delete c;
//    });
//    d->backgroundOffsetTop->start();
    after();
}

void MainScreen::hideSplash() {
    if (d->splashHidden) return;
    d->splashHidden = true;

    QGraphicsOpacityEffect* stackOpacity = new QGraphicsOpacityEffect();
    ui->stackedWidget->setGraphicsEffect(stackOpacity);
    stackOpacity->setOpacity(0);

    tVariantAnimation* stackAnim = new tVariantAnimation(this);
    stackAnim->setStartValue(1.0);
    stackAnim->setEndValue(0.0);
    stackAnim->setDuration(250);
    stackAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(stackAnim, &tVariantAnimation::valueChanged, this, [ = ](QVariant value) {
        stackOpacity->setOpacity(value.toReal());
    });
    connect(stackAnim, &tVariantAnimation::finished, this, [ = ] {
        if (stackAnim->direction() == tVariantAnimation::Forward) {
            ui->stackedWidget->setCurrentWidget(ui->menuPage);
            d->hudOpacity->start();

            stackAnim->setDirection(tVariantAnimation::Backward);
            stackAnim->start();
        } else {
            ui->menuPage->setFocus();
            stackAnim->deleteLater();
        }
    });
    stackAnim->start();
}

bool MainScreen::event(QEvent* event) {
    if (event->type() == GamepadEvent::type()) {
        GamepadEvent* gamepadEvent = static_cast<GamepadEvent*>(event);
        if (gamepadEvent->buttonPressed() && gamepadEvent->button() == QGamepadManager::ButtonA) {
            hideSplash();
        }
    }
    return QWidget::event(event);
}

void MainScreen::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) hideSplash();
}

void MainScreen::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) hideSplash();
}

void MainScreen::on_informationButton_clicked() {
    CreditsScreen* cred = new CreditsScreen(this);
    connect(cred, &CreditsScreen::done, cred, &CreditsScreen::deleteLater);
}

void MainScreen::on_settingsButton_clicked() {
    emit goToSettings();
}
