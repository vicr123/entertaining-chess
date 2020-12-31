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
#ifndef TURNBROWSER_H
#define TURNBROWSER_H

#include <QWidget>
#include "game/gameengine.h"

namespace Ui {
    class TurnBrowser;
}

class GameEngine;
struct TurnBrowserPrivate;
class TurnBrowser : public QWidget {
        Q_OBJECT

    public:
        explicit TurnBrowser(QWidget* parent = nullptr);
        ~TurnBrowser();

        QSize sizeHint() const;

        void setEngine(GameEnginePtr engine);

        void nextTurn();
        void previousTurn();

    signals:
        void currentTurnChanged(int turn);

    private:
        TurnBrowserPrivate* d;
        Ui::TurnBrowser* ui;

        // QWidget interface
    protected:
        void paintEvent(QPaintEvent* event);
        void resizeEvent(QResizeEvent* event);

        // QObject interface
    public:
        bool event(QEvent* event);
};

#endif // TURNBROWSER_H
