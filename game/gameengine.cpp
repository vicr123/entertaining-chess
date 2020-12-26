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
#include "gameengine.h"

#include <QPoint>
#include "MoveEngines/abstractmoveengine.h"
#include "MoveEngines/humanmoveengine.h"

struct GameEnginePrivate {
    AbstractMoveEngine* player1, *player2;
    bool isPlayer1Turn = true;

    QList<int> boardLayout = {
        10,  8,  9, 11, 12,  9,  8, 10,
        7,  7,  7,  7,  7,  7,  7,  7,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        1,  1,  1,  1,  1,  1,  1,  1,
        4,  2,  3,  5,  6,  3,  2,  4
    };
    QList<GameEngine::Piece> takenPieces;

    bool whiteKingsideCastlePossible = true;
    bool whiteQueensidesideCastlePossible = true;
    bool blackKingsideCastlePossible = true;
    bool blackQueensidesideCastlePossible = true;
};

GameEngine::GameEngine(AbstractMoveEngine* player1, AbstractMoveEngine* player2, QObject* parent) : QObject(parent) {
    d = new GameEnginePrivate();
    d->player1 = player1;
    d->player2 = player2;

    player1->setGameEngine(this);
    player2->setGameEngine(this);

    player1->startTurn();
}

GameEngine::~GameEngine() {
    delete d;
}

int GameEngine::gridToIndex(int x, int y) {
    return y * 8 + x;
}

QPoint GameEngine::indexToGrid(int index) {
    return {index % 8, index / 8};
}

void GameEngine::issueMove(int from, int to) {
    if (!isValidMove(from, to)) return;

    Piece fromPiece = pieceAt(from);

    //Move the piece
    MoveResults results = issueMove(from, to, d->boardLayout);
    d->boardLayout = results.newBoardLayout;
    if (results.takenPiece != Empty) d->takenPieces.append(results.takenPiece);

    //TODO: Check if a rook is taken and update the castling flag accordingly
    if (isWhiteTurn()) {
        if (fromPiece == WhiteKing) {
            d->whiteKingsideCastlePossible = false;
            d->whiteQueensidesideCastlePossible = false;
        } else if (fromPiece == WhiteRook) {
            if (pieceAt(gridToIndex(0, 7)) != WhiteRook) d->whiteQueensidesideCastlePossible = false;
            if (pieceAt(gridToIndex(7, 7)) != WhiteRook) d->whiteKingsideCastlePossible = false;
        }
    } else {
        if (fromPiece == BlackKing) {
            d->blackKingsideCastlePossible = false;
            d->blackQueensidesideCastlePossible = false;
        } else if (fromPiece == BlackRook) {
            if (pieceAt(gridToIndex(0, 0)) != BlackRook) d->blackQueensidesideCastlePossible = false;
            if (pieceAt(gridToIndex(7, 0)) != BlackRook) d->blackKingsideCastlePossible = false;
        }
    }

    emit moveIssued(from, to, d->isPlayer1Turn);

    d->isPlayer1Turn = !d->isPlayer1Turn;
    if (d->isPlayer1Turn) {
        d->player1->startTurn();
    } else {
        d->player2->startTurn();
    }
}

GameEngine::MoveResults GameEngine::issueMove(int from, int to, QList<int> boardLayout) {
    auto pieceAt = [ = ](int index) {
        return static_cast<Piece>(boardLayout.at(index));
    };

    Piece fromPiece = pieceAt(from);
    Piece toPiece = pieceAt(to);

    MoveResults results;

    results.takenPiece = toPiece;
    boardLayout.replace(to, d->boardLayout.at(from));
    boardLayout.replace(from, 0);

    //Check for a castle
    if (fromPiece == WhiteKing && from == 60) {
        if (d->whiteQueensidesideCastlePossible && to == 58) {
            //Queenside Castle
            boardLayout = issueMove(56, 59, boardLayout).newBoardLayout;
        }
        if (d->whiteKingsideCastlePossible && to == 62) {
            //Kingside Castle
            boardLayout = issueMove(63, 61, boardLayout).newBoardLayout;
        }
    } else if (fromPiece == BlackKing && from == 4) {
        if (d->blackQueensidesideCastlePossible && to == 2) {
            //Queenside Castle
            boardLayout = issueMove(0, 3, boardLayout).newBoardLayout;
        }
        if (d->blackKingsideCastlePossible && to == 6) {
            //Kingside Castle
            boardLayout = issueMove(7, 5, boardLayout).newBoardLayout;
        }
    }

    results.newBoardLayout = boardLayout;

    //TODO: Handle En Passant

    return results;
}

bool GameEngine::isValidMove(int from, int to) {
    return isValidMove(from, to, d->boardLayout, isWhiteTurn());
}

bool GameEngine::isValidMove(int from, int to, QList<int> boardLayout, bool isWhiteTurn, bool checkForCheckCondition) {
    auto pieceAt = [ = ](int index) {
        return static_cast<Piece>(boardLayout.at(index));
    };

    QPoint fromPoint = indexToGrid(from);
    QPoint toPoint = indexToGrid(to);
    Piece fromPiece = pieceAt(from);
    Piece toPiece = pieceAt(to);

    if (isWhiteTurn && fromPiece > WhitePiecesEnd) return false; //Can't move other player's piece
    if (!isWhiteTurn && fromPiece <= WhitePiecesEnd) return false; //Can't move other player's piece
    if (isOwnPiece(isWhiteTurn, toPiece)) return false; //Can't take own piece

    //TODO: Ensure that this move (if legal) will not place the player's king in check
    if (checkForCheckCondition) {
        //Move the piece
        QList<int> temporaryBoardLayout = boardLayout;
        temporaryBoardLayout.replace(to, temporaryBoardLayout.at(from));
        temporaryBoardLayout.replace(from, 0);
        if (isCheck(isWhiteTurn, temporaryBoardLayout)) return false;
    }

    switch (fromPiece) {
        case GameEngine::WhitePawn: {
            //TODO: Calculate En Passant

            if (toPiece == Empty) {
                //Advance two squares if on rank 2
                bool canAdvanceTwoSquares = fromPoint.y() == 6;
                if (fromPoint.x() == toPoint.x() && fromPoint.y() == toPoint.y() + 1) return true; //Pawn can move by one
                if (fromPoint.x() == toPoint.x() && fromPoint.y() == toPoint.y() + 2 && canAdvanceTwoSquares) return true; //Pawn can move by two
                return false; //Cannot move any other way otherwise
            } else {
                if (fromPoint.x() != toPoint.x() - 1 && fromPoint.x() != toPoint.x() + 1) return false; //Pawn can't move horizontally, unless taking another piece
                if (fromPoint.y() != toPoint.y() + 1) return false; //Pawn can only take one step ahead
                return true; //Taking a piece
            }
        }
        case BlackPawn: {
            //TODO: Calculate En Passant

            if (toPiece == Empty) {
                //Advance two squares if on rank 7
                bool canAdvanceTwoSquares = fromPoint.y() == 1;
                if (fromPoint.x() == toPoint.x() && fromPoint.y() == toPoint.y() - 1) return true; //Pawn can move by one
                if (fromPoint.x() == toPoint.x() && fromPoint.y() == toPoint.y() - 2 && canAdvanceTwoSquares) return true; //Pawn can move by two
                return false; //Cannot move any other way otherwise
            } else {
                if (fromPoint.x() != toPoint.x() - 1 && fromPoint.x() != toPoint.x() + 1) return false; //Pawn can't move horizontally, unless taking another piece
                if (fromPoint.y() != toPoint.y() - 1) return false; //Pawn can only take one step ahead
                return true; //Taking a piece
            }
        }
        case GameEngine::WhiteKnight:
        case GameEngine::BlackKnight: {
            if (((fromPoint.x() == toPoint.x() + 1 || fromPoint.x() == toPoint.x() - 1) && (fromPoint.y() == toPoint.y() + 2 || fromPoint.y() == toPoint.y() - 2)) ||
                ((fromPoint.x() == toPoint.x() + 2 || fromPoint.x() == toPoint.x() - 2) && (fromPoint.y() == toPoint.y() + 1 || fromPoint.y() == toPoint.y() - 1))) {
                //...it's just how a knight moves
                return true;
            }
            return false;
        }
        case GameEngine::WhiteBishop:
        case GameEngine::BlackBishop: {
            //Move diagonally until collision
            if (fromPoint.x() != 0 && fromPoint.y() != 0) {
                for (int i = from - 9; ; i -= 9) {
                    //Diagonally to the top left
                    if (i < 0 || i > 63) break;
                    if (to == i) return true;
                    if (pieceAt(i) != Empty) break; //We can move here but no further
                    if (indexToGrid(i).x() == 0) break; //We can not move further to the left
                    if (indexToGrid(i).y() == 0) break; //We can not move further to the top
                }
            }

            if (fromPoint.x() != 7 && fromPoint.y() != 0) {
                for (int i = from - 7; ; i -= 7) {
                    //Diagonally to the top right
                    if (i < 0 || i > 63) break;
                    if (to == i) return true;
                    if (pieceAt(i) != Empty) break; //We can move here but no further
                    if (indexToGrid(i).x() == 7) break; //We can not move further to the right
                    if (indexToGrid(i).y() == 0) break; //We can not move further to the top
                }
            }

            if (fromPoint.x() != 7 && fromPoint.y() != 7) {
                for (int i = from + 9; ; i += 9) {
                    //Diagonally to the bottom right
                    if (i < 0 || i > 63) break;
                    if (to == i) return true;
                    if (pieceAt(i) != Empty) break; //We can move here but no further
                    if (indexToGrid(i).x() == 7) break; //We can not move further to the right
                    if (indexToGrid(i).y() == 7) break; //We can not move further to the bottom
                }
            }

            if (fromPoint.x() != 0 && fromPoint.y() != 7) {
                for (int i = from + 7; ; i += 7) {
                    //Diagonally to the bottom left
                    if (i < 0 || i > 63) break;
                    if (to == i) return true;
                    if (pieceAt(i) != Empty) break; //We can move here but no further
                    if (indexToGrid(i).x() == 0) break; //We can not move further to the right
                    if (indexToGrid(i).y() == 7) break; //We can not move further to the bottom
                }
            }

            return false;
        }
        case GameEngine::WhiteRook:
        case GameEngine::BlackRook: {
            //Move laterally until collision
            if (fromPoint.x() != 0) {
                for (int i = from - 1; ; i -= 1) {
                    //Left
                    if (i < 0 || i > 63) break;
                    if (to == i) return true;
                    if (pieceAt(i) != Empty) break; //We can move here but no further
                    if (indexToGrid(i).x() == 0) break; //We can not move further to the left
                }
            }

            if (fromPoint.x() != 7) {
                for (int i = from + 1; ; i += 1) {
                    //Right
                    if (i < 0 || i > 63) break;
                    if (to == i) return true;
                    if (pieceAt(i) != Empty) break; //We can move here but no further
                    if (indexToGrid(i).x() == 7) break; //We can not move further to the right
                }
            }

            if (fromPoint.y() != 7) {
                for (int i = from + 8; ; i += 8) {
                    //Down
                    if (i < 0 || i > 63) break;
                    if (to == i) return true;
                    if (pieceAt(i) != Empty) break; //We can move here but no further
                    if (indexToGrid(i).y() == 7) break; //We can not move further to the bottom
                }
            }

            if (fromPoint.y() != 0) {
                for (int i = from - 8; ; i -= 8) {
                    //Up
                    if (i < 0 || i > 63) break;
                    if (to == i) return true;
                    if (pieceAt(i) != Empty) break; //We can move here but no further
                    if (indexToGrid(i).y() == 0) break; //We can not move further to the bottom
                }
            }

            return false;
        }
        case GameEngine::WhiteQueen: {
            //Turn the queen into a rook and bishop
            boardLayout.replace(from, WhiteRook);
            if (isValidMove(from, to, boardLayout, isWhiteTurn)) return true;
            boardLayout.replace(from, WhiteBishop);
            if (isValidMove(from, to, boardLayout, isWhiteTurn)) return true;
            return false;
        }
        case BlackQueen: {
            //Turn the queen into a rook and bishop
            boardLayout.replace(from, BlackRook);
            if (isValidMove(from, to, boardLayout, isWhiteTurn)) return true;
            boardLayout.replace(from, BlackBishop);
            if (isValidMove(from, to, boardLayout, isWhiteTurn)) return true;
            return false;
        }
        case GameEngine::WhiteKing:
        case GameEngine::BlackKing: {
            if (fromPoint.x() >= toPoint.x() - 1 && fromPoint.x() <= toPoint.x() + 1 && fromPoint.y() >= toPoint.y() - 1 && fromPoint.y() <= toPoint.y() + 1) {
                return true;
            }

            //Don't need to check from position because if the king moves, he can no longer castle anyway
            if (fromPiece == WhiteKing) {
                if (d->whiteQueensidesideCastlePossible && to == 58) {
                    //Ensure the spaces are empty
                    if (pieceAt(57) != Empty || pieceAt(58) != Empty || pieceAt(59) != Empty) return false;

                    //Ensure the king is not walking through a conflicting square
                    boardLayout.replace(59, WhiteKing);
                    boardLayout.replace(60, Empty);
                    if (isCheck(true, boardLayout)) return false;

                    return true;
                }
                if (d->whiteKingsideCastlePossible && to == 62) {
                    if (pieceAt(61) != Empty || pieceAt(62) != Empty) return false;

                    //Ensure the king is not walking through a conflicting square
                    boardLayout.replace(61, WhiteKing);
                    boardLayout.replace(60, Empty);
                    if (isCheck(true, boardLayout)) return false;

                    return true;
                }
            } else {
                if (d->blackQueensidesideCastlePossible && to == 2) {
                    //Ensure the spaces are empty
                    if (pieceAt(1) != Empty || pieceAt(2) != Empty || pieceAt(3) != Empty) return false;

                    //Ensure the king is not walking through a conflicting square
                    boardLayout.replace(2, BlackKing);
                    boardLayout.replace(4, Empty);
                    if (isCheck(false, boardLayout)) return false;

                    return true;
                }
                if (d->blackKingsideCastlePossible && to == 6)  {
                    if (pieceAt(5) != Empty || pieceAt(6) != Empty) return false;

                    //Ensure the king is not walking through a conflicting square
                    boardLayout.replace(5, BlackKing);
                    boardLayout.replace(4, Empty);
                    if (isCheck(false, boardLayout)) return false;

                    return true;
                }
            }

            return false;
        }
        case Empty:
            return false;

    }

    return true;
}

AbstractMoveEngine* GameEngine::engineForCurrentTurn() {
    return d->isPlayer1Turn ? d->player1 : d->player2;
}

bool GameEngine::isHumanTurn() {
    return qobject_cast<HumanMoveEngine*>(engineForCurrentTurn()) != 0;
}

bool GameEngine::isWhiteTurn() {
    return engineForCurrentTurn() == d->player1;
}

bool GameEngine::isCheck() {
    return isCheck(isWhiteTurn(), d->boardLayout);
}

bool GameEngine::isCheck(bool isWhiteCheck, QList<int> boardLayout) {
    auto pieceAt = [ = ](int index) {
        return static_cast<Piece>(boardLayout.at(index));
    };

    //Locate the king
    int otherKing = -1;
    for (int i = 0; i < 64; i++) {
        if ((isWhiteCheck && pieceAt(i) == WhiteKing) || (!isWhiteCheck && pieceAt(i) == BlackKing)) {
            otherKing = i;
            break;
        }
    }

    //If we can't locate the other king, either something has gone horribly wrong
    //or we're just checking before a board move
    if (otherKing == -1) return false;

    //See if any piece can take the king
    for (int i = 0; i < 64; i++) {
        Piece piece = pieceAt(i);
        if ((isWhiteCheck && !isWhitePiece(piece)) || (!isWhiteCheck && isWhitePiece(piece))) {
            if (isValidMove(i, otherKing, boardLayout, !isWhiteCheck, false)) return true;
        }
    }
    return false;
}

bool GameEngine::isOwnPiece(GameEngine::Piece piece) {
    return isOwnPiece(isWhiteTurn(), piece);
}

bool GameEngine::isOwnPiece(bool isWhiteTurn, GameEngine::Piece piece) {
    if (piece == Empty) return false;
    if (isWhiteTurn && isWhitePiece(piece)) return true;
    if (!isWhiteTurn && !isWhitePiece(piece)) return true;
    return false;
}

bool GameEngine::isWhitePiece(GameEngine::Piece piece) {
    return piece <= WhitePiecesEnd && piece != Empty;
}

GameEngine::Piece GameEngine::pieceAt(int index) {
    return static_cast<Piece>(d->boardLayout.at(index));
}
