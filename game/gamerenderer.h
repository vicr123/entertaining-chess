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
#ifndef GAMERENDERER_H
#define GAMERENDERER_H

#include <QWidget>
#include "gameengine.h"

struct GameRendererPrivate;
class GameRenderer : public QWidget {
        Q_OBJECT
    public:
        explicit GameRenderer(QWidget* parent = nullptr);
        ~GameRenderer();

        void setGameEngine(GameEnginePtr engine);
        GameEnginePtr gameEngine();

        void setFixedGameState(int turn);

        QRect viewport();

        void moveSelection(Qt::Edge direction);
        void select();

        bool canSelect();
        bool canIssueMove();
        bool canPutDown();

    signals:
        void buttonsChanged();
        void pauseRequested();

    private:
        GameRendererPrivate* d;

        void updateBoardRotation();
        bool shouldRotateToWhite();

        // QWidget interface
    protected:
        void mousePressEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);
        void mouseMoveEvent(QMouseEvent* event);
        void keyPressEvent(QKeyEvent* event);
        void leaveEvent(QEvent* event);
        void paintEvent(QPaintEvent* event);

        // QObject interface
    public:
        bool event(QEvent* event);
};

#endif // GAMERENDERER_H
