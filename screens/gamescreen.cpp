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
#include <QKeyEvent>

GameScreen::GameScreen(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::GameScreen) {
    ui->setupUi(this);

    ui->gamepadHud->bindKey(Qt::Key_Escape, QGamepadManager::ButtonStart);

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonStart, [ = ] {
        ui->menuButton->click();
    });

    connect(ui->gameRenderer, &GameRenderer::buttonsChanged, this, [ = ] {
        ui->gamepadHud->removeText(QGamepadManager::ButtonA);
        ui->gamepadHud->removeText(QGamepadManager::ButtonB);

        if (ui->gameRenderer->canPutDown()) {
            ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Put Down"));
        } else {
            ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
        }

        if (ui->gameRenderer->canIssueMove()) ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Issue Move"));
    });
    connect(ui->gameRenderer, &GameRenderer::pauseRequested, this, [ = ] {
        ui->menuButton->click();
    });

    this->setFocusProxy(ui->gameRenderer);
}

GameScreen::~GameScreen() {
    delete ui;
}

void GameScreen::setGameEngine(GameEnginePtr engine) {
    ui->gameRenderer->setGameEngine(engine);
    connect(engine.data(), &GameEngine::endOfGame, this, [ = ](GameEngine::GameResult result) {
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
                endgameScreen->setDescriptionLabelText(tr("This game cannot be won!"));
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
    connect(engine.data(), &GameEngine::currentTurnChanged, this, [ = ](bool isWhiteTurn) {
        if (isWhiteTurn) {
            ui->whitePlayerWidget->setIsActive(true);
            ui->blackPlayerWidget->setIsActive(false);
        } else {
            ui->whitePlayerWidget->setIsActive(false);
            ui->blackPlayerWidget->setIsActive(true);
        }

        if (engine->isHumanTurn()) {
            if (engine->isCheck()) {
                MusicEngine::playSoundEffect("yourturn-check");
            } else {
                MusicEngine::playSoundEffect("yourturn");
            }
        }
    });
    if (engine->isWhiteTurn()) {
        ui->whitePlayerWidget->setIsActive(true);
        ui->blackPlayerWidget->setIsActive(false);
    } else {
        ui->whitePlayerWidget->setIsActive(false);
        ui->blackPlayerWidget->setIsActive(true);
    }
}

void GameScreen::setPlayers(QImage whitePicture, QString whiteName, QImage blackPicture, QString blackName) {
    ui->whitePlayerWidget->setPlayerDetails(whitePicture, whiteName);
    ui->blackPlayerWidget->setPlayerDetails(blackPicture, blackName);
}

void GameScreen::on_menuButton_clicked() {
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
    connect(screen, &PauseScreen::settings, this, [ = ] {
        screen->deleteLater();
        emit viewSettings();
    });
    screen->show();
}


void GameScreen::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        ui->menuButton->click();
    }
}
