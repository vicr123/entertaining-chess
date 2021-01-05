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
#ifndef CREATEPRIVATEGAMESCREEN_H
#define CREATEPRIVATEGAMESCREEN_H

#include <QWidget>
#include "game/gameengine.h"

namespace Ui {
    class CreatePrivateGameScreen;
}

class GameEngine;
struct CreatePrivateGameScreenPrivate;
class CreatePrivateGameScreen : public QWidget {
        Q_OBJECT

    public:
        explicit CreatePrivateGameScreen(QWidget* parent = nullptr);
        ~CreatePrivateGameScreen();

    private slots:
        void on_titleLabel_backButtonClicked();

        void on_existingGameButton_clicked();

        void on_newGameButton_clicked();

        void on_readyButton_clicked();

        void on_startButton_clicked();

    signals:
        void done();
        void startGame(GameEnginePtr engine);
        void setPlayers(QImage whiteImage, QString whiteName, QImage blackImage, QString blackName);

    private:
        CreatePrivateGameScreenPrivate* d;
        Ui::CreatePrivateGameScreen* ui;

        void close();

        // QObject interface
    public:
        bool eventFilter(QObject* watched, QEvent* event);
};

#endif // CREATEPRIVATEGAMESCREEN_H
