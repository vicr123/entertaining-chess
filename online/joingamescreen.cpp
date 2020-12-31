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
#include "joingamescreen.h"
#include "ui_joingamescreen.h"

#include "onlinecontroller.h"
#include <online/onlinewebsocket.h>
#include <pauseoverlay.h>
#include <online/onlineapi.h>
#include <QJsonObject>
#include <questionoverlay.h>
#include "game/gameengine.h"
#include "game/MoveEngines/humanmoveengine.h"
#include "game/MoveEngines/onlinemoveengine.h"

struct JoinGameScreenPrivate {
    QImage profilePicture;
    QString username;

    QImage peerProfilePicture;
    QString peerUsername;
    QByteArray gameData;
    bool isPlayerWhite;
    bool isInformationAvailable = false;
};

JoinGameScreen::JoinGameScreen(QString gameCode, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::JoinGameScreen) {
    ui->setupUi(this);

    d = new JoinGameScreenPrivate();

    ui->subtitleLabel->setFixedWidth(SC_DPI(600));
    ui->readyButton->setFixedWidth(SC_DPI(600));

    PauseOverlay::overlayForWindow(this)->pushOverlayWidget(this);

    OnlineApi::instance()->get("/users/profile")->then([ = ](QJsonDocument doc) {
        QJsonObject obj = doc.object().value("user").toObject();

        //Get the profile picture
        int pictureSize = SC_DPI(64);
        OnlineApi::instance()->profilePicture(obj.value("gravHash").toString(), pictureSize)->then([ = ](QImage image) {
            d->profilePicture = image;
            d->username = obj.value("username").toString();
        });
    })->error([ = ](QString error) {

    });

    ui->titleLabel->setBackButtonShown(true);

    connect(OnlineController::instance()->ws(), &OnlineWebSocket::jsonMessageReceived, this, [ = ](QJsonDocument json) {
        QJsonObject object = json.object();
        QString type = object.value("type").toString();
        if (type == "gameDetails") {
            d->isPlayerWhite = object.value("isPlayerWhite").toBool();
            d->isInformationAvailable = true;

            QJsonValue savedGame = object.value("savedGame");
            if (savedGame.type() == QJsonValue::String) {
                d->gameData = QByteArray::fromBase64(savedGame.toString().toUtf8());
            }

            this->switchToStaging();
        } else if (type == "peerConnected") {
            int pictureSize = SC_DPI(64);
            OnlineApi::instance()->profilePicture(object.value("picture").toString(), pictureSize)->then([ = ](QImage image) {
                d->peerProfilePicture = image;
                d->peerUsername = object.value("username").toString();

                this->switchToStaging();
            });
        } else if (type == "peerConnectionError") {
            PauseOverlay::overlayForWindow(this)->popOverlayWidget([ = ] {
                QuestionOverlay* question = new QuestionOverlay(this);
                question->setIcon(QMessageBox::Critical);
                question->setTitle(tr("Invalid Game Code"));
                question->setText(tr("Sorry, that game code doesn't point to a game. Check the game code and try again."));
                question->setButtons(QMessageBox::Ok);
                connect(question, &QuestionOverlay::accepted, question, &QuestionOverlay::deleteLater);
                connect(question, &QuestionOverlay::rejected, question, &QuestionOverlay::deleteLater);
                emit done();
            });
        } else if (type == "peerDisconnected") {
            PauseOverlay::overlayForWindow(this)->popOverlayWidget([ = ] {
                emit done();
            });
        } else if (type == "startGame") {
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
    });

    QTimer::singleShot(1000, this, [ = ] {
        OnlineController::instance()->ws()->sendJsonO({
            {"type", "joinPrivate"},
            {"code", gameCode}
        });
    });
}

JoinGameScreen::~JoinGameScreen() {
    delete d;
    delete ui;
}

void JoinGameScreen::on_titleLabel_backButtonClicked() {
    OnlineController::instance()->ws()->sendJsonO({
        {"type", "disconnectPeer"}
    });
}

void JoinGameScreen::close() {

}

void JoinGameScreen::switchToStaging() {
    if (!d->isInformationAvailable || d->peerUsername.isEmpty()) return;
    if (d->isPlayerWhite) {
        ui->battlePlayers->setWhiteSide(d->profilePicture, d->username);
        ui->battlePlayers->setBlackSide(d->peerProfilePicture, d->peerUsername);
    } else {
        ui->battlePlayers->setWhiteSide(d->peerProfilePicture, d->peerUsername);
        ui->battlePlayers->setBlackSide(d->profilePicture, d->username);
    }
    ui->stackedWidget->setCurrentWidget(ui->preparingPage);
}

void JoinGameScreen::on_readyButton_clicked() {
    ui->readyButton->setEnabled(false);
    OnlineController::instance()->ws()->sendJsonO({
        {"type", "peerReady"}
    });
}
