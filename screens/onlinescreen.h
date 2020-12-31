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
#ifndef ONLINESCREEN_H
#define ONLINESCREEN_H

#include <QWidget>
#include "game/gameengine.h"

namespace Ui {
    class OnlineScreen;
}

struct OnlineScreenPrivate;
class GameEngine;
class OnlineScreen : public QWidget {
        Q_OBJECT

    public:
        explicit OnlineScreen(QWidget* parent = nullptr);
        ~OnlineScreen();

    signals:
        void quitOnline();

    private slots:
        void on_stackedWidget_currentChanged(int arg1);

    private:
        OnlineScreenPrivate* d;
        Ui::OnlineScreen* ui;

        void startGame(GameEnginePtr engine);
};

#endif // ONLINESCREEN_H
