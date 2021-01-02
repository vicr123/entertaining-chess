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
#include "onlinecontroller.h"

#include <questionoverlay.h>
#include <online/onlinewebsocket.h>
#include <online/onlineapi.h>
#include <musicengine.h>

struct OnlineControllerPrivate {
    QWidget* mainWindow;
    OnlineWebSocket* ws;

    bool isOnline = false;
    bool isHost = false;
    bool havePeer = false;
};

OnlineController::OnlineController(QObject* parent) : QObject(parent) {
    d = new OnlineControllerPrivate();
}

OnlineController* OnlineController::instance() {
    static OnlineController* i = new OnlineController();
    return i;
}

void OnlineController::setMainWindow(QWidget* mainWindow) {
    d->mainWindow = mainWindow;
}

void OnlineController::connectToOnline() {
    emit onlineStateChanged(Connecting);
    d->isOnline = true;
    emit isOnlineChanged(true);
    OnlineApi::instance()->play("EntertainingChess", "1.0", d->mainWindow)->then([ = ](OnlineWebSocket * ws) {
        emit onlineStateChanged(Idle);
        d->ws = ws;

        connect(ws, &OnlineWebSocket::disconnected, this, [ = ] {
            QuestionOverlay::StandardDialog error;
            switch (ws->closeCode()) {
                case QWebSocketProtocol::CloseCodeNormal:
                    d->isOnline = false;
                    emit isOnlineChanged(false);
                    return;
                case QWebSocketProtocol::CloseCodeGoingAway:
                    error = QuestionOverlay::ServerMaintenanceAboutToStart;
                    break;
                case QWebSocketProtocol::CloseCodeProtocolError:
                    error = QuestionOverlay::ServerProtocolError;
                    break;
                case QWebSocketProtocol::CloseCodeDatatypeNotSupported:
                case QWebSocketProtocol::CloseCodeReserved1004:
                case QWebSocketProtocol::CloseCodeMissingStatusCode:
                case QWebSocketProtocol::CloseCodeWrongDatatype:
                case QWebSocketProtocol::CloseCodePolicyViolated:
                case QWebSocketProtocol::CloseCodeTooMuchData:
                case QWebSocketProtocol::CloseCodeAbnormalDisconnection:
                default:
                    error = QuestionOverlay::ServerDisconnected;
            }

            QuestionOverlay* question = new QuestionOverlay(d->mainWindow);
            question->setStandardDialog(error);

            auto handler = [ = ] {
                question->deleteLater();
                d->isOnline = false;
                emit isOnlineChanged(false);
            };

            connect(question, &QuestionOverlay::accepted, this, handler);
            connect(question, &QuestionOverlay::rejected, this, handler);
        });

        connect(ws, &OnlineWebSocket::jsonMessageReceived, this, [ = ](QJsonDocument json) {
            QJsonObject object = json.object();
            QString type = object.value("type").toString();
            if (type == "peerConnected") {
                d->isHost = object.value("isHost").toBool();
                d->havePeer = true;
            } else if (type == "peerDisconnected") {
                d->havePeer = false;
                emit onlineStateChanged(Idle);

                QString reason = object.value("reason").toString();
                if (reason == "userForfeit") {
                    QuestionOverlay* question = new QuestionOverlay(d->mainWindow);
                    question->setIcon(QMessageBox::Information);
                    question->setTitle(tr("Disconnected"));
                    question->setText(tr("Your opponent has left the match."));
                    question->setButtons(QMessageBox::Ok);

                    auto handler = [ = ] {

                    };

                    connect(question, &QuestionOverlay::accepted, this, handler);
                    connect(question, &QuestionOverlay::rejected, this, handler);
                }
            }
        });

        //TODO: Decode Discord Join Secret
    })->error([ = ](QString error) {
        //TODO: Clear Discord join secret

        if (error == "disconnect") {
            d->isOnline = false;
            emit isOnlineChanged(false);
            return;
        }

        QuestionOverlay* question = new QuestionOverlay(d->mainWindow);
        question->setIcon(QMessageBox::Critical);
        question->setTitle(tr("Error"));
        question->setText(OnlineApi::errorFromPromiseRejection(error));
        question->setButtons(QMessageBox::Ok);

        auto handler = [ = ] {
            question->deleteLater();
            d->isOnline = false;
            emit isOnlineChanged(false);
        };

        connect(question, &QuestionOverlay::accepted, this, handler);
        connect(question, &QuestionOverlay::rejected, this, handler);
    });
}

void OnlineController::disconnectFromOnline() {
    d->ws->close();
}

bool OnlineController::isOnline() {
    return d->isOnline;
}

bool OnlineController::isHost() {
    return d->isHost;
}

bool OnlineController::havePeer() {
    return d->havePeer;
}

OnlineWebSocket* OnlineController::ws() {
    return d->ws;
}
