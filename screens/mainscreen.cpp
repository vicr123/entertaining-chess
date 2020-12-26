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

#include "game/gameengine.h"
#include "game/MoveEngines/humanmoveengine.h"
#include <musicengine.h>
#include <loadoverlay.h>
#include <the-libs_global.h>

MainScreen::MainScreen(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::MainScreen) {
    ui->setupUi(this);

    ui->spacer1->changeSize(0, SC_DPI(200), QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->topBarrier->setBounceWidget(ui->playButton);
    ui->bottomBarrier->setBounceWidget(ui->exitButton);
    ui->exitButton->setProperty("type", "destructive");

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Exit"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [ = ] {
        ui->exitButton->click();
    });
}

MainScreen::~MainScreen() {
    delete ui;
}

void MainScreen::on_exitButton_clicked() {
    QApplication::exit();
}

void MainScreen::on_loadButton_clicked() {
    MusicEngine::playSoundEffect(MusicEngine::Selection);

    LoadOverlay* load = new LoadOverlay(this);
    connect(load, &LoadOverlay::loadData, this, [ = ](QDataStream * stream) {
//        loadGame(stream);
    });
    load->load();
}

void MainScreen::on_playButton_clicked() {
    emit startGame(new GameEngine(new HumanMoveEngine(), new HumanMoveEngine()));
}
