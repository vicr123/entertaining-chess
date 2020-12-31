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
#include "createprivategamescreen.h"
#include "ui_createprivategamescreen.h"

#include <loadoverlay.h>
#include <pauseoverlay.h>
#include <online/onlineapi.h>
#include <online/onlinecontroller.h>
#include <online/onlinewebsocket.h>
#include <questionoverlay.h>
#include "game/gameengine.h"
#include "game/MoveEngines/humanmoveengine.h"
#include "game/MoveEngines/onlinemoveengine.h"

struct CreatePrivateGameScreenPrivate {
    QByteArray gameData;

    QImage profilePicture;
    QString username;

    bool isReady = false;
    bool isPlayerWhite;
};

CreatePrivateGameScreen::CreatePrivateGameScreen(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::CreatePrivateGameScreen) {
    ui->setupUi(this);

    d = new CreatePrivateGameScreenPrivate();

    ui->titleLabel->setBackButtonShown(true);
    ui->mainWidget->setFixedWidth(SC_DPI(600));
    ui->joiningWidget->setFixedWidth(SC_DPI(600));
    ui->challengerWidget->setFixedWidth(SC_DPI(600));
    ui->joiningWidget->setVisible(false);
    ui->challengerWidget->setVisible(false);
    PauseOverlay::overlayForWindow(parent)->pushOverlayWidget(this);


    //Load data from the server
    OnlineApi::instance()->get("/users/profile")->then([ = ](QJsonDocument doc) {
        QJsonObject obj = doc.object().value("user").toObject();

        //Get the profile picture
        int pictureSize = SC_DPI(64);
        OnlineApi::instance()->profilePicture(obj.value("gravHash").toString(), pictureSize)->then([ = ](QImage image) {
            d->profilePicture = image;
            d->username = obj.value("username").toString();
            ui->readyButton->setEnabled(true);
        });
    })->error([ = ](QString error) {

    });

    connect(OnlineController::instance()->ws(), &OnlineWebSocket::jsonMessageReceived, this, [ = ](QJsonDocument json) {
        QJsonObject object = json.object();
        QString type = object.value("type").toString();
        if (type == "matchmakingStarted") {
            d->isReady = true;
            ui->readyButton->setText(tr("Cancel"));
            ui->gameSettingsWidget->setEnabled(false);
            d->isPlayerWhite = object.value("playerIsWhite").toBool();

            if (d->isPlayerWhite) {
                ui->battlePlayers->setWhiteSide(d->profilePicture, d->username);
            } else {
                ui->battlePlayers->setBlackSide(d->profilePicture, d->username);
            }


            ui->joinCode->setText(object.value("code").toString());
            ui->joiningWidget->setVisible(true);
        } else if (type == "matchmakingCancelled" || type == "peerDisconnected") {
            d->isReady = false;
            ui->readyButton->setText(tr("Ready!"));
            ui->gameSettingsWidget->setEnabled(true);
            ui->battlePlayers->clearWhiteSide();
            ui->battlePlayers->clearBlackSide();
            ui->joiningWidget->setVisible(false);
            ui->challengerWidget->setVisible(false);
        } else if (type == "peerConnected") {
            QString peerUsername = object.value("username").toString();
            int pictureSize = SC_DPI(64);
            OnlineApi::instance()->profilePicture(object.value("picture").toString(), pictureSize)->then([ = ](QImage image) {
                if (d->isPlayerWhite) {
                    ui->battlePlayers->setBlackSide(image, peerUsername);
                } else {
                    ui->battlePlayers->setWhiteSide(image, peerUsername);
                }
            });

            QJsonValue savedGame;
            if (!d->gameData.isEmpty()) {
                savedGame = QString(d->gameData.toBase64());
            }

            OnlineController::instance()->ws()->sendJsonO({
                {"type", "gameDetails"},
                {"isPlayerWhite", !d->isPlayerWhite},
                {"savedGame", savedGame}
            });

            ui->joiningWidget->setVisible(false);
            ui->challengerWidget->setVisible(true);
            ui->challengerMessage->setText(tr("You'll be facing off with %1.").arg(peerUsername));
            ui->startButton->setEnabled(false);
        } else if (type == "peerReady") {
            ui->startButton->setEnabled(true);
        } else if (type == "peerNotReady") {
            ui->startButton->setEnabled(false);
        }
    });
}

CreatePrivateGameScreen::~CreatePrivateGameScreen() {
    delete d;
    delete ui;
}

void CreatePrivateGameScreen::on_titleLabel_backButtonClicked() {
    if (d->isReady) {
        ui->readyButton->click();
    } else {
        close();
    }
}

void CreatePrivateGameScreen::close() {
    PauseOverlay::overlayForWindow(this)->popOverlayWidget([ = ] {
        emit done();
    });
}

void CreatePrivateGameScreen::on_existingGameButton_clicked() {
    auto loadCancelledHandler = [ = ] {
        if (d->gameData.isEmpty()) {
            ui->existingGameButton->setChecked(false);
            ui->newGameButton->setChecked(true);
        } else {
            ui->existingGameButton->setChecked(true);
            ui->newGameButton->setChecked(false);
        }
    };

    LoadOverlay* load = new LoadOverlay(this);
    connect(load, &LoadOverlay::loadData, this, [ = ](QDataStream * stream) {
        QByteArray gameData = stream->device()->readAll();
        if (GameEngine::isGameCorrupted(gameData)) {
            QuestionOverlay* question = new QuestionOverlay(this);
            question->setIcon(QMessageBox::Critical);
            question->setTitle(tr("Corrupt File"));
            question->setText(tr("Sorry, that file is corrupt and needs to be deleted."));
            question->setButtons(QMessageBox::Ok);
            connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
            connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);

            loadCancelledHandler();
        } else {
            d->gameData = gameData;
            ui->existingGameButton->setChecked(true);
            ui->newGameButton->setChecked(false);
        }
    });
    connect(load, &LoadOverlay::canceled, this, loadCancelledHandler);
    load->load();
}

void CreatePrivateGameScreen::on_newGameButton_clicked() {
    d->gameData.clear();
    ui->existingGameButton->setChecked(false);
    ui->newGameButton->setChecked(true);
}

void CreatePrivateGameScreen::on_readyButton_clicked() {
    if (d->isReady) {
        //Cancel ready
        OnlineController::instance()->ws()->sendJsonO({
            {"type", "cancelMatchmaking"}
        });
    } else {
        //Get ready!
        bool isWhite;
        if (ui->playWhite->isChecked()) {
            isWhite = true;
        } else if (ui->playBlack->isChecked()) {
            isWhite = false;
        } else {
            isWhite = QRandomGenerator::system()->bounded(2);
        }

        OnlineController::instance()->ws()->sendJsonO({
            {"type", "launchPrivate"},
            {"playerIsWhite", isWhite}
        });
    }

}

void CreatePrivateGameScreen::on_startButton_clicked() {
    OnlineController::instance()->ws()->sendJsonO({
        {"type", "startGame"}
    });

    //Start the game!
    GameEnginePtr engine;
    if (d->isPlayerWhite) {
        engine.reset(new GameEngine(new HumanMoveEngine, new OnlineMoveEngine));
    } else {
        engine.reset(new GameEngine(new OnlineMoveEngine, new HumanMoveEngine));
    }

    if (!d->gameData.isEmpty()) {
        QDataStream dataStream(&d->gameData, QIODevice::ReadOnly);
        engine->loadGame(&dataStream);
    } else {
        engine->startGame();
    }

    emit startGame(engine);
    PauseOverlay::overlayForWindow(this)->popOverlayWidget([ = ] {
        emit done();
    });
}
