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
#include "pausescreen.h"
#include "ui_pausescreen.h"

#include <musicengine.h>
#include <pauseoverlay.h>
#include <questionoverlay.h>
#include <saveoverlay.h>
#include "game/gameengine.h"

struct PauseScreenPrivate {
    GameEnginePtr engine;
};

PauseScreen::PauseScreen(GameEnginePtr engine, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::PauseScreen) {
    ui->setupUi(this);

    d = new PauseScreenPrivate();
    d->engine = engine;

    PauseOverlay::overlayForWindow(this->parentWidget())->pushOverlayWidget(this);

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    if (engine->isMoveAvailable()) ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Resume"));
    ui->gamepadHud->bindKey(Qt::Key_Left, QGamepadManager::ButtonL1);
    ui->gamepadHud->bindKey(Qt::Key_Right, QGamepadManager::ButtonR1);

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonL1, [ = ] {
        previousScreen();
    });
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonR1, [ = ] {
        nextScreen();
    });
    if (engine->isMoveAvailable()) {
        ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [ = ] {
            ui->resumeButton->click();
        });
        ui->gamepadHud->setButtonAction(QGamepadManager::ButtonStart, [ = ] {
            ui->resumeButton->click();
        });
    }

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Fade);
    ui->stackedWidget->currentChanged(0);

    ui->mainMenuButton->setProperty("type", "destructive");

    QPalette pal = ui->stackedWidget->palette();
    pal.setColor(QPalette::Window, Qt::transparent);
    ui->stackedWidget->setPalette(pal);

    ui->gameRewindRenderer->setGameEngine(engine);
    ui->turnBrowser->setEngine(engine);
    connect(ui->turnBrowser, &TurnBrowser::currentTurnChanged, this, [ = ](int index) {
        ui->gameRewindRenderer->setFixedGameState(index);
    });

    if (!engine->isMoveAvailable()) {
        ui->resumeButton->setVisible(false);
        ui->saveButton->setVisible(false);
    }

    this->setFocusProxy(ui->turnBrowser);
}

PauseScreen::~PauseScreen() {
    delete ui;
    delete d;
}

void PauseScreen::previousScreen() {
    int screen = ui->stackedWidget->currentIndex() - 1;
    if (screen < 0) screen = 0;
    ui->stackedWidget->setCurrentIndex(screen);
}

void PauseScreen::nextScreen() {
    int screen = ui->stackedWidget->currentIndex() + 1;
    if (screen == ui->stackedWidget->count()) screen = ui->stackedWidget->count() - 1;
    ui->stackedWidget->setCurrentIndex(screen);
}

void PauseScreen::on_resumeButton_clicked() {
    MusicEngine::playSoundEffect(MusicEngine::Backstep);
    MusicEngine::playBackgroundMusic();
    PauseOverlay::overlayForWindow(this->parentWidget())->popOverlayWidget([ = ] {
        emit resume();
    });
}

void PauseScreen::on_saveButton_clicked() {
    MusicEngine::playSoundEffect(MusicEngine::Selection);

    SaveOverlay* save = new SaveOverlay(this);
    connect(save, &SaveOverlay::provideMetadata, this, [ = ](QVariantMap * metadata) {
        //TODO
        QStringList description;
        description.append("A chess game");

        metadata->insert("description", description.join(" ∙ "));
    });
    connect(save, &SaveOverlay::provideSaveData, this, [ = ](QDataStream * stream) {
        d->engine->saveGame(stream);
    });
    save->save();
}

void PauseScreen::on_mainMenuButton_clicked() {
//    QuestionOverlay* overlay = new QuestionOverlay(this);
//    overlay->setButtons(QMessageBox::Yes | QMessageBox::Cancel, tr("Abandon Game"), true);
//    overlay->setTitle(tr("Abandon this game?"));
//    overlay->setText(tr("Unless you've saved this game, you won't be able to continue it."));
//    overlay->setIcon(QMessageBox::Question);
//    connect(overlay, &QuestionOverlay::accepted, this, [ = ](QMessageBox::StandardButton button) {
//        if (button == QMessageBox::Yes) {
    //Return to the main menu
    MusicEngine::playSoundEffect(MusicEngine::Selection);
    PauseOverlay::overlayForWindow(this->parentWidget())->popOverlayWidget([ = ] {
        emit mainMenu();
    });
//        }
//    });
//    connect(overlay, &QuestionOverlay::rejected, overlay, &QuestionOverlay::deleteLater);
}

void PauseScreen::on_stackedWidget_currentChanged(int arg1) {
    switch (arg1) {
        case 0:
            ui->gamepadHud->removeText(QGamepadManager::ButtonL1);
            ui->gamepadHud->setButtonText(QGamepadManager::ButtonR1, tr("System"));
            break;
        case 1:
            ui->gamepadHud->setButtonText(QGamepadManager::ButtonL1, tr("Move History"));
            ui->gamepadHud->removeText(QGamepadManager::ButtonR1);
            break;
    }
}

void PauseScreen::on_leftButton_clicked() {
    previousScreen();
}

void PauseScreen::on_rightButton_clicked() {
    nextScreen();
}

