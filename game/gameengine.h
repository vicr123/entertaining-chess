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
#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <QObject>

class AbstractMoveEngine;
struct GameEnginePrivate;
class GameEngine : public QObject {
        Q_OBJECT
    public:
        explicit GameEngine(AbstractMoveEngine* player1, AbstractMoveEngine* player2, QObject* parent = nullptr);
        ~GameEngine();

        enum Piece {
            Empty = 0,
            WhitePawn = 1,
            WhiteKnight = 2,
            WhiteBishop = 3,
            WhiteRook = 4,
            WhiteQueen = 5,
            WhiteKing = 6,
            WhitePiecesEnd = WhiteKing,
            BlackPawn = 7,
            BlackKnight = 8,
            BlackBishop = 9,
            BlackRook = 10,
            BlackQueen = 11,
            BlackKing = 12,
            BlackPiecesEnd = BlackKing
        };

        struct MoveResults {
            QList<int> newBoardLayout;
            Piece takenPiece;
        };

        int gridToIndex(int x, int y);
        QPoint indexToGrid(int index);

        void issueMove(int from, int to);
        MoveResults issueMove(int from, int to, QList<int> boardLayout);
        bool isValidMove(int from, int to);
        bool isValidMove(int from, int to, QList<int> boardLayout, bool isWhiteTurn, bool checkForCheckCondition = true);

        AbstractMoveEngine* engineForCurrentTurn();
        bool isHumanTurn();
        bool isWhiteTurn();
        bool isCheck();
        bool isCheck(bool isWhiteCheck, QList<int> boardLayout);
        bool isOwnPiece(Piece piece);
        bool isOwnPiece(bool isWhiteTurn, Piece piece);
        static bool isWhitePiece(Piece piece);

        Piece pieceAt(int index);

    signals:
        void moveIssued(int from, int to, bool isPlayer1);

    private:
        GameEnginePrivate* d;
};

#endif // GAMEENGINE_H
