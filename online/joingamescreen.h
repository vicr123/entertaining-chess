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
#ifndef JOINGAMESCREEN_H
#define JOINGAMESCREEN_H

#include <QWidget>
#include "game/gameengine.h"

namespace Ui {
    class JoinGameScreen;
}

class GameEngine;
struct JoinGameScreenPrivate;
class JoinGameScreen : public QWidget {
        Q_OBJECT

    public:
        explicit JoinGameScreen(QString gameCode, QWidget* parent = nullptr);
        ~JoinGameScreen();

    private slots:
        void on_titleLabel_backButtonClicked();

        void on_readyButton_clicked();

    signals:
        void done();
        void startGame(GameEnginePtr engine);
        void setPlayers(QImage whiteImage, QString whiteName, QImage blackImage, QString blackName);

    private:
        JoinGameScreenPrivate* d;
        Ui::JoinGameScreen* ui;

        void close();
        void switchToStaging();
};

#endif // JOINGAMESCREEN_H
