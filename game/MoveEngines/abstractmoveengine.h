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
#ifndef ABSTRACTMOVEENGINE_H
#define ABSTRACTMOVEENGINE_H

#include <QObject>

class GameEngine;
struct AbstractMoveEnginePrivate;
class AbstractMoveEngine : public QObject {
        Q_OBJECT
    public:
        explicit AbstractMoveEngine(QObject* parent = nullptr);
        ~AbstractMoveEngine();

        void setGameEngine(GameEngine* engine);
        GameEngine* gameEngine();

        virtual void startTurn() = 0;

    signals:

    private:
        AbstractMoveEnginePrivate* d;
};

#endif // ABSTRACTMOVEENGINE_H
