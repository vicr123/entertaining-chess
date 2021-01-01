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
#include "onlinescreen.h"
#include "ui_onlinescreen.h"

#include <QJsonObject>
#include <online/onlinewebsocket.h>
#include "online/onlinecontroller.h"
#include "controllers/backgroundcontroller.h"

struct OnlineScreenPrivate {
    GameEnginePtr engine = nullptr;
};

OnlineScreen::OnlineScreen(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::OnlineScreen) {
    ui->setupUi(this);

    d = new OnlineScreenPrivate();

    this->setFocusProxy(ui->stackedWidget);

    connect(OnlineController::instance(), &OnlineController::onlineStateChanged, this, [ = ](OnlineController::OnlineState state) {
        //Don't do anything during the endgame
        if (d->engine && !d->engine->isMoveAvailable()) return;
        switch (state) {
            case OnlineController::Connecting:
                ui->stackedWidget->setCurrentWidget(ui->connectingPage);
                break;
            case OnlineController::Idle:
                ui->stackedWidget->setCurrentWidget(ui->mainMenuPage);
                break;
        }
    });

    connect(ui->mainMenuPage, &OnlineMainMenu::startGame, this, &OnlineScreen::startGame);
    connect(ui->gamePage, &GameScreen::returnToMainMenu, this, [ = ] {
        if (OnlineController::instance()->havePeer()) {
            //Disconnect the peer
            OnlineController::instance()->ws()->sendJsonO({
                {"type", "disconnectPeer"},
                {"reason", "userForfeit"}
            });
        }

        ui->stackedWidget->setCurrentWidget(ui->mainMenuPage);
    });

    BackgroundController::instance()->install(ui->connectingPage);
}

OnlineScreen::~OnlineScreen() {
    delete d;
    delete ui;
}

void OnlineScreen::startGame(GameEnginePtr engine) {
    d->engine = engine;
    ui->gamePage->setGameEngine(engine);
    ui->stackedWidget->setCurrentWidget(ui->gamePage);
}

void OnlineScreen::on_stackedWidget_currentChanged(int arg1) {
    ui->stackedWidget->widget(arg1)->setFocus();
}
