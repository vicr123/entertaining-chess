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
#include "gamescreen.h"
#include "ui_gamescreen.h"

#include <musicengine.h>
#include "game/gameengine.h"
#include "endgamescreen.h"
#include "pausescreen.h"
#include <pauseoverlay.h>
#include <QTimer>

GameScreen::GameScreen(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::GameScreen) {
    ui->setupUi(this);

    ui->gamepadHud->bindKey(Qt::Key_Escape, QGamepadManager::ButtonStart);

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonStart, [ = ] {
        ui->menuButton->click();
    });
}

GameScreen::~GameScreen() {
    delete ui;
}

void GameScreen::setGameEngine(GameEngine* engine) {
    ui->gameRenderer->setGameEngine(engine);
    connect(engine, &GameEngine::endOfGame, this, [ = ](GameEngine::GameResult result) {
        EndgameScreen* endgameScreen = new EndgameScreen();
        switch (result) {
            case GameEngine::WhiteWins:
                endgameScreen->setMainLabelText(tr("White wins!"));
                endgameScreen->setDescriptionLabelText(tr("Black is in check and is unable to escape."));
                break;
            case GameEngine::BlackWins:
                endgameScreen->setMainLabelText(tr("Black wins!"));
                endgameScreen->setDescriptionLabelText(tr("White is in check and is unable to escape."));
                break;
            case GameEngine::Stalemate:
                endgameScreen->setMainLabelText(tr("Stalemate!"));
                endgameScreen->setDescriptionLabelText(tr("Add a spiffy description here!"));
                break;
        }

        PauseOverlay::overlayForWindow(this)->pushOverlayWidget(endgameScreen);
        QTimer::singleShot(3000, this, [ = ] {
            PauseOverlay::overlayForWindow(this)->popOverlayWidget([ = ] {
                ui->menuButton->click();
                endgameScreen->deleteLater();
            });
        });
    });
}

void GameScreen::on_menuButton_clicked() {
    MusicEngine::pauseBackgroundMusic();
    MusicEngine::playSoundEffect(MusicEngine::Pause);

    PauseScreen* screen = new PauseScreen(ui->gameRenderer->gameEngine(), this);
    connect(screen, &PauseScreen::resume, this, [ = ] {
        screen->deleteLater();
        ui->gameRenderer->setFocus();
    });
    connect(screen, &PauseScreen::mainMenu, this, [ = ] {
        screen->deleteLater();
        emit returnToMainMenu();
    });
//    connect(screen, &PauseScreen::provideMetadata, this, [ = ](QVariantMap * metadata) {
//        //TODO
//        QStringList description;
//        description.append(tr("Chess Game"));

//        metadata->insert("description", description.join(" ∙ "));
//    });
//    connect(screen, &PauseScreen::provideSaveData, this, [ = ](QDataStream * data) {
//        ui->gameRenderer->gameEngine()->saveGame(data);
//    });
    screen->show();
}
