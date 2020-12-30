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
    emit isOnlineChanged(true);
    OnlineApi::instance()->play("EntertainingChess", "1.0", d->mainWindow)->then([ = ](OnlineWebSocket * ws) {
        emit onlineStateChanged(Idle);
        d->ws = ws;

        connect(ws, &OnlineWebSocket::disconnected, this, [ = ] {
            QString error;
            switch (ws->closeCode()) {
                case QWebSocketProtocol::CloseCodeNormal:
                    break;
                case QWebSocketProtocol::CloseCodeGoingAway:
                    error = tr("You've been disconnected because the server is about to undergo maintenance.");
                    break;
                case QWebSocketProtocol::CloseCodeProtocolError:
                    error = tr("You've been disconnected from the server because there was a communication error.");
                    break;
                case QWebSocketProtocol::CloseCodeDatatypeNotSupported:
                case QWebSocketProtocol::CloseCodeReserved1004:
                case QWebSocketProtocol::CloseCodeMissingStatusCode:
                case QWebSocketProtocol::CloseCodeWrongDatatype:
                case QWebSocketProtocol::CloseCodePolicyViolated:
                case QWebSocketProtocol::CloseCodeTooMuchData:
                case QWebSocketProtocol::CloseCodeAbnormalDisconnection:
                default:
                    error = tr("You've been disconnected from the server.");
            }

            if (error.isEmpty()) {
                emit isOnlineChanged(false);
            } else {
                QuestionOverlay* question = new QuestionOverlay(d->mainWindow);
                question->setIcon(QMessageBox::Critical);
                question->setTitle(tr("Error"));
                question->setText(error);
                question->setButtons(QMessageBox::Ok);

                auto handler = [ = ] {
                    question->deleteLater();
                    emit isOnlineChanged(false);
                };

                connect(question, &QuestionOverlay::accepted, this, handler);
                connect(question, &QuestionOverlay::rejected, this, handler);


                //Stop any music
                MusicEngine::pauseBackgroundMusic();
            }
        });

        //TODO: Decode Discord Join Secret
    })->error([ = ](QString error) {
        //TODO: Clear Discord join secret

        if (error == "disconnect") {
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
            emit isOnlineChanged(false);
        };

        connect(question, &QuestionOverlay::accepted, this, handler);
        connect(question, &QuestionOverlay::rejected, this, handler);
    });
}

void OnlineController::disconnectFromOnline() {
    d->ws->close();
}

OnlineWebSocket* OnlineController::ws() {
    return d->ws;
}
