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
#include "onlinemoveengine.h"

#include <QTimer>
#include <online/onlinewebsocket.h>
#include "online/onlinecontroller.h"
#include "game/gameengine.h"
#include <QJsonObject>

OnlineMoveEngine::OnlineMoveEngine(QObject* parent) : AbstractMoveEngine(parent) {
    connect(OnlineController::instance()->ws(), &OnlineWebSocket::jsonMessageReceived, this, [ = ](QJsonDocument message) {
        QJsonObject object = message.object();
        QString type = object.value("type").toString();
        if (type == "pieceMoved") {
            if (gameEngine()->isWhiteTurn() == isWhite()) {
                int from = object.value("from").toInt();
                int to = object.value("to").toInt();
                GameEngine::Piece promoteTo = static_cast<GameEngine::Piece>(object.value("promoteTo").toInt());

                gameEngine()->issueMove(from, to, promoteTo);
            }
        }
    });
    QTimer::singleShot(0, this, [ = ] {
        connect(gameEngine().data(), &GameEngine::moveIssued, this, [ = ](int from, int to, GameEngine::Piece promoteTo, bool isWhiteMove) {
            if (!isWhiteMove == isWhite()) {
                OnlineController::instance()->ws()->sendJsonO({
                    {"type", "pieceMoved"},
                    {"from", from},
                    {"to", to},
                    {"promoteTo", static_cast<int>(promoteTo)}
                });
            }
        });
    });
}

void OnlineMoveEngine::startTurn() {

}
