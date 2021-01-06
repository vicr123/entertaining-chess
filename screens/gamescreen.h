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
#ifndef GAMESCREEN_H
#define GAMESCREEN_H

#include <QWidget>
#include "game/gameengine.h"

namespace Ui {
    class GameScreen;
}

class GameScreen : public QWidget {
        Q_OBJECT

    public:
        explicit GameScreen(QWidget* parent = nullptr);
        ~GameScreen();

        void setGameEngine(GameEnginePtr engine);

        void setPlayers(QImage whitePicture, QString whiteName, QImage blackPicture, QString blackName);

    signals:
        void returnToMainMenu();
        void viewSettings();

    private slots:
        void on_menuButton_clicked();

    private:
        Ui::GameScreen* ui;

        // QWidget interface
    protected:
        void keyPressEvent(QKeyEvent* event);
};

#endif // GAMESCREEN_H
