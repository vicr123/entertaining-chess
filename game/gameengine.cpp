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
#include <QStack>
#include <QMap>
#include <QDataStream>
#include "MoveEngines/abstractmoveengine.h"
#include "MoveEngines/humanmoveengine.h"

QDataStream& operator<<(QDataStream& dataStream, const GameEngine::Piece& piece) {
    dataStream << static_cast<quint32>(piece);
    return dataStream;
}

QDataStream& operator>>(QDataStream& dataStream, GameEngine::Piece& piece) {
    quint32 temp;
    dataStream >> temp;
    piece = static_cast<GameEngine::Piece>(temp);
    return dataStream;
}

QDataStream& operator<<(QDataStream& dataStream, const GameEngine::GameMode& gameMode) {
    dataStream << static_cast<quint32>(gameMode);
    return dataStream;
}

QDataStream& operator>>(QDataStream& dataStream, GameEngine::GameMode& gameMode) {
    quint32 temp;
    dataStream >> temp;
    gameMode = static_cast<GameEngine::GameMode>(temp);
    return dataStream;
}

QDataStream& operator<<(QDataStream& dataStream, const GameEngine::MoveType& moveType) {
    dataStream << static_cast<quint32>(moveType);
    return dataStream;
}

QDataStream& operator>>(QDataStream& dataStream, GameEngine::MoveType& moveType) {
    quint32 temp;
    dataStream >> temp;
    moveType = static_cast<GameEngine::MoveType>(temp);
    return dataStream;
}

struct GameEnginePrivate {
    AbstractMoveEngine* player1, *player2;
    bool isPlayer1Turn = true;

    QList<quint8> boardLayout;
    QList<GameEngine::Piece> takenPieces;

    //These fields are used for temporary rewinding of the game state
    QList<quint8> currentBoardLayout;
    QList<GameEngine::Piece> currentTakenPieces;

    QStack<GameEngine::MoveResults> previousMoves;

    GameEngine::GameMode gameMode = GameEngine::StandardGameMode;
    bool whiteKingsideCastlePossible = true;
    bool whiteQueensidesideCastlePossible = true;
    bool blackKingsideCastlePossible = true;
    bool blackQueensidesideCastlePossible = true;
    quint8 enpassant = -1;
};

GameEngine::GameEngine(AbstractMoveEngine* player1, AbstractMoveEngine* player2, QObject* parent) : QObject(parent) {
    d = new GameEnginePrivate();
    d->player1 = player1;
    d->player2 = player2;

    d->boardLayout = {
        10,  8,  9, 11, 12,  9,  8, 10,
        7,  7,  7,  7,  7,  7,  7,  7,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        1,  1,  1,  1,  1,  1,  1,  1,
        4,  2,  3,  5,  6,  3,  2,  4
    };

    player1->setGameEngine(this);
    player2->setGameEngine(this);
}

GameEngine::~GameEngine() {
    delete d;
}

void GameEngine::startGame() {
    d->boardLayout = {
        10,  8,  9, 11, 12,  9,  8, 10,
        7,  7,  7,  7,  7,  7,  7,  7,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        1,  1,  1,  1,  1,  1,  1,  1,
        4,  2,  3,  5,  6,  3,  2,  4
    };
    d->takenPieces.clear();
    d->previousMoves.clear();
    d->whiteKingsideCastlePossible = true;
    d->whiteQueensidesideCastlePossible = true;
    d->blackKingsideCastlePossible = true;
    d->blackQueensidesideCastlePossible = true;
    d->enpassant = -1;

    MoveResults initialBoardState;
    initialBoardState.newBoardLayout = d->boardLayout;
    initialBoardState.moveType = InitialBoardState;
    d->previousMoves.append(initialBoardState);

    d->player1->startTurn();
}

void GameEngine::saveGame(QDataStream* data) {
    *data << d->gameMode;
    *data << d->isPlayer1Turn;
    for (int i = 0; i < 64; i++) {
        *data << d->boardLayout.at(i);
    }

    *data << static_cast<quint32>(d->takenPieces.length());
    for (Piece piece : qAsConst(d->takenPieces)) {
        *data << piece;
    }

    quint8 gameFlags = static_cast<quint8>(d->blackKingsideCastlePossible) | static_cast<quint8>(d->blackQueensidesideCastlePossible) << 1 | static_cast<quint8>(d->whiteKingsideCastlePossible) << 2 | static_cast<quint8>(d->whiteKingsideCastlePossible) << 3;
    *data << gameFlags;
    *data << d->enpassant;
    *data << static_cast<quint32>(d->previousMoves.length());
    for (const MoveResults& results : qAsConst(d->previousMoves)) {
        *data << results.from;
        *data << results.to;
        *data << results.moveType;
        *data << results.movedPiece;
        *data << results.takenPiece;

        *data << static_cast<quint32>(results.takenPieces.length());
        for (Piece piece : results.takenPieces) {
            *data << piece;
        }
        for (int i = 0; i < 64; i++) {
            *data << results.newBoardLayout.at(i);
        }
    }
}

bool GameEngine::loadGame(QDataStream* data) {
    *data >> d->gameMode;
    *data >> d->isPlayer1Turn;
    for (int i = 0; i < 64; i++) {
        quint8 boardIndex;
        *data >> boardIndex;
        d->boardLayout.replace(i, boardIndex);
    }

    quint32 takenPiecesLength;
    *data >> takenPiecesLength;
    if (takenPiecesLength > 2048) return false;
    for (quint32 i = 0; i < takenPiecesLength; i++) {
        Piece piece;
        *data >> piece;
        d->takenPieces.append(piece);
    }

    quint8 gameFlags;
    *data >> gameFlags;
    d->blackKingsideCastlePossible = gameFlags & 1;
    d->blackQueensidesideCastlePossible = gameFlags & 2;
    d->whiteKingsideCastlePossible = gameFlags & 4;
    d->whiteQueensidesideCastlePossible = gameFlags & 8;
    *data >> d->enpassant;

    quint32 previousMovesLength;
    *data >> previousMovesLength;
    if (previousMovesLength > 2048) return false;
    for (quint32 i = 0; i < previousMovesLength; i++) {
        MoveResults results;
        *data >> results.from;
        *data >> results.to;
        *data >> results.moveType;
        *data >> results.movedPiece;
        *data >> results.takenPiece;

        quint32 takenPiecesLength;
        *data >> takenPiecesLength;
        if (takenPiecesLength > 2048) return false;
        for (quint32 j = 0; j < takenPiecesLength; j++) {
            Piece piece;
            *data >> piece;
            results.takenPieces.append(piece);
        }
        for (int j = 0; j < 64; j++) {
            quint8 boardIndex;
            *data >> boardIndex;
            results.newBoardLayout.append(boardIndex);
        }

        d->previousMoves.append(results);
    }

    if (data->status() != QDataStream::Ok) return false;

    if (d->isPlayer1Turn) {
        d->player1->startTurn();
    } else {
        d->player2->startTurn();
    }

    return true;
}

QStack<GameEngine::MoveResults> GameEngine::previousMoves() {
    return d->previousMoves;
}

int GameEngine::gridToIndex(int x, int y) {
    return y * 8 + x;
}

QPoint GameEngine::indexToGrid(int index) {
    return {index % 8, index / 8};
}

QString GameEngine::indexToCoordinate(int index) {
    int file = index % 8;
    int rank = index / 8;

    return QStringLiteral("%1%2").arg(QStringLiteral("abcdefgh").at(file)).arg(QStringLiteral("87654321").at(rank));
}

void GameEngine::issueMove(int from, int to, Piece promoteTo) {
    if (!isValidMove(from, to)) return;

    Piece fromPiece = pieceAt(from);

    //Move the piece
    MoveResults results = issueMove(from, to, d->boardLayout, true, promoteTo);
    d->boardLayout = results.newBoardLayout;
    d->takenPieces = results.takenPieces;

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

    d->previousMoves.append(results);
    emit moveIssued(from, to, d->isPlayer1Turn);

    d->isPlayer1Turn = !d->isPlayer1Turn;

    if (isMoveAvailable()) {
        if (d->isPlayer1Turn) {
            d->player1->startTurn();
        } else {
            d->player2->startTurn();
        }
    } else {
        //End of game!
        if (isCheck()) {
            if (isWhiteTurn()) {
                emit endOfGame(BlackWins);
            } else {
                emit endOfGame(WhiteWins);
            }
        } else {
            emit endOfGame(Stalemate);
        }
    }
}

GameEngine::MoveResults GameEngine::issueMove(int from, int to, QList<quint8> boardLayout, bool animate, Piece promoteTo) {
    auto pieceAt = [ = ](int index) {
        return static_cast<Piece>(boardLayout.at(index));
    };

    Piece fromPiece = pieceAt(from);
    Piece toPiece = pieceAt(to);

    MoveResults results;
    results.takenPieces = d->takenPieces;

    results.from = from;
    results.to = to;
    results.movedPiece = fromPiece;
    results.takenPiece = toPiece;
    results.moveType = StandardMove;
    boardLayout.replace(to, d->boardLayout.at(from));
    boardLayout.replace(from, 0);
    if (toPiece != Empty) results.takenPieces.append(results.takenPiece);

    //Check for a castle
    if (fromPiece == WhiteKing && from == 60) {
        if (d->whiteQueensidesideCastlePossible && to == 58) {
            //Queenside Castle
            boardLayout = issueMove(56, 59, boardLayout, animate).newBoardLayout;
            results.moveType = QueensideCastle;
        }
        if (d->whiteKingsideCastlePossible && to == 62) {
            //Kingside Castle
            boardLayout = issueMove(63, 61, boardLayout, animate).newBoardLayout;
            results.moveType = KingsideCastle;
        }
    } else if (fromPiece == BlackKing && from == 4) {
        if (d->blackQueensidesideCastlePossible && to == 2) {
            //Queenside Castle
            boardLayout = issueMove(0, 3, boardLayout, animate).newBoardLayout;
            results.moveType = QueensideCastle;
        }
        if (d->blackKingsideCastlePossible && to == 6) {
            //Kingside Castle
            boardLayout = issueMove(7, 5, boardLayout, animate).newBoardLayout;
            results.moveType = KingsideCastle;
        }
    }

    //Check for a promotion
    if (fromPiece == WhitePawn && to <= 7) {
        boardLayout.replace(to, promoteTo);
        results.moveType = Promotion;
        results.movedPiece = promoteTo;
    } else if (fromPiece == BlackPawn && to >= 56) {
        boardLayout.replace(to, promoteTo);
        results.moveType = Promotion;
        results.movedPiece = promoteTo;
    }

    results.newBoardLayout = boardLayout;

    //TODO: Handle En Passant

    if (animate) emit animatePiece(from, to, fromPiece);

    return results;
}

bool GameEngine::isValidMove(int from, int to) {
    return isValidMove(from, to, d->boardLayout, isWhiteTurn());
}

bool GameEngine::isValidMove(int from, int to, QList<quint8> boardLayout, bool isWhiteTurn, bool checkForCheckCondition) {
    QPoint fromPoint = indexToGrid(from);
    QPoint toPoint = indexToGrid(to);
    Piece fromPiece = pieceAt(from, boardLayout);
    Piece toPiece = pieceAt(to, boardLayout);

    if (isWhiteTurn && fromPiece > WhitePiecesEnd) return false; //Can't move other player's piece
    if (!isWhiteTurn && fromPiece <= WhitePiecesEnd) return false; //Can't move other player's piece
    if (isOwnPiece(isWhiteTurn, toPiece)) return false; //Can't take own piece

    if (checkForCheckCondition) {
        //Move the piece
        QList<quint8> temporaryBoardLayout = boardLayout;
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
                    if (pieceAt(i, boardLayout) != Empty) break; //We can move here but no further
                    if (indexToGrid(i).x() == 0) break; //We can not move further to the left
                    if (indexToGrid(i).y() == 0) break; //We can not move further to the top
                }
            }

            if (fromPoint.x() != 7 && fromPoint.y() != 0) {
                for (int i = from - 7; ; i -= 7) {
                    //Diagonally to the top right
                    if (i < 0 || i > 63) break;
                    if (to == i) return true;
                    if (pieceAt(i, boardLayout) != Empty) break; //We can move here but no further
                    if (indexToGrid(i).x() == 7) break; //We can not move further to the right
                    if (indexToGrid(i).y() == 0) break; //We can not move further to the top
                }
            }

            if (fromPoint.x() != 7 && fromPoint.y() != 7) {
                for (int i = from + 9; ; i += 9) {
                    //Diagonally to the bottom right
                    if (i < 0 || i > 63) break;
                    if (to == i) return true;
                    if (pieceAt(i, boardLayout) != Empty) break; //We can move here but no further
                    if (indexToGrid(i).x() == 7) break; //We can not move further to the right
                    if (indexToGrid(i).y() == 7) break; //We can not move further to the bottom
                }
            }

            if (fromPoint.x() != 0 && fromPoint.y() != 7) {
                for (int i = from + 7; ; i += 7) {
                    //Diagonally to the bottom left
                    if (i < 0 || i > 63) break;
                    if (to == i) return true;
                    if (pieceAt(i, boardLayout) != Empty) break; //We can move here but no further
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
                    if (pieceAt(i, boardLayout) != Empty) break; //We can move here but no further
                    if (indexToGrid(i).x() == 0) break; //We can not move further to the left
                }
            }

            if (fromPoint.x() != 7) {
                for (int i = from + 1; ; i += 1) {
                    //Right
                    if (i < 0 || i > 63) break;
                    if (to == i) return true;
                    if (pieceAt(i, boardLayout) != Empty) break; //We can move here but no further
                    if (indexToGrid(i).x() == 7) break; //We can not move further to the right
                }
            }

            if (fromPoint.y() != 7) {
                for (int i = from + 8; ; i += 8) {
                    //Down
                    if (i < 0 || i > 63) break;
                    if (to == i) return true;
                    if (pieceAt(i, boardLayout) != Empty) break; //We can move here but no further
                    if (indexToGrid(i).y() == 7) break; //We can not move further to the bottom
                }
            }

            if (fromPoint.y() != 0) {
                for (int i = from - 8; ; i -= 8) {
                    //Up
                    if (i < 0 || i > 63) break;
                    if (to == i) return true;
                    if (pieceAt(i, boardLayout) != Empty) break; //We can move here but no further
                    if (indexToGrid(i).y() == 0) break; //We can not move further to the bottom
                }
            }

            return false;
        }
        case GameEngine::WhiteQueen: {
            //Turn the queen into a rook and bishop
            boardLayout.replace(from, WhiteRook);
            if (isValidMove(from, to, boardLayout, isWhiteTurn, checkForCheckCondition)) return true;
            boardLayout.replace(from, WhiteBishop);
            if (isValidMove(from, to, boardLayout, isWhiteTurn, checkForCheckCondition)) return true;
            return false;
        }
        case BlackQueen: {
            //Turn the queen into a rook and bishop
            boardLayout.replace(from, BlackRook);
            if (isValidMove(from, to, boardLayout, isWhiteTurn, checkForCheckCondition)) return true;
            boardLayout.replace(from, BlackBishop);
            if (isValidMove(from, to, boardLayout, isWhiteTurn, checkForCheckCondition)) return true;
            return false;
        }
        case GameEngine::WhiteKing:
        case GameEngine::BlackKing: {
            if (fromPoint.x() >= toPoint.x() - 1 && fromPoint.x() <= toPoint.x() + 1 && fromPoint.y() >= toPoint.y() - 1 && fromPoint.y() <= toPoint.y() + 1) {
                return true;
            }

            //Don't need to check from position because if the king moves, he can no longer castle anyway
            if (checkForCheckCondition && !isCheck(isWhiteTurn, boardLayout)) {
                if (fromPiece == WhiteKing) {
                    if (d->whiteQueensidesideCastlePossible && to == 58) {
                        //Ensure the spaces are empty
                        if (pieceAt(57, boardLayout) != Empty || pieceAt(58, boardLayout) != Empty || pieceAt(59, boardLayout) != Empty) return false;

                        //Ensure the king is not walking through a conflicting square
                        boardLayout.replace(59, WhiteKing);
                        boardLayout.replace(60, Empty);
                        if (isCheck(true, boardLayout)) return false;

                        return true;
                    }
                    if (d->whiteKingsideCastlePossible && to == 62) {
                        if (pieceAt(61, boardLayout) != Empty || pieceAt(62, boardLayout) != Empty) return false;

                        //Ensure the king is not walking through a conflicting square
                        boardLayout.replace(61, WhiteKing);
                        boardLayout.replace(60, Empty);
                        if (isCheck(true, boardLayout)) return false;

                        return true;
                    }
                } else {
                    if (d->blackQueensidesideCastlePossible && to == 2) {
                        //Ensure the spaces are empty
                        if (pieceAt(1, boardLayout) != Empty || pieceAt(2, boardLayout) != Empty || pieceAt(3, boardLayout) != Empty) return false;

                        //Ensure the king is not walking through a conflicting square
                        boardLayout.replace(2, BlackKing);
                        boardLayout.replace(4, Empty);
                        if (isCheck(false, boardLayout)) return false;

                        return true;
                    }
                    if (d->blackKingsideCastlePossible && to == 6)  {
                        if (pieceAt(5, boardLayout) != Empty || pieceAt(6, boardLayout) != Empty) return false;

                        //Ensure the king is not walking through a conflicting square
                        boardLayout.replace(5, BlackKing);
                        boardLayout.replace(4, Empty);
                        if (isCheck(false, boardLayout)) return false;

                        return true;
                    }
                }
            }

            return false;
        }
        case Empty:
            return false;

    }

    return true;
}

bool GameEngine::isMoveAvailable() {
    //Go through each piece and see if a move for the current colour is available
    for (int i = 0; i < 64; i++) {
        Piece piece = pieceAt(i);
        if ((isWhiteTurn() && isWhitePiece(piece)) || (!isWhiteTurn() && isBlackPiece(piece))) {
            //Check if this piece has any moves
            for (int j = 0; j < 64; j++) {
                if (isValidMove(i, j)) return true;
            }
        }
    }
    return false;
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

bool GameEngine::isCheck(bool isWhiteCheck, QList<quint8> boardLayout) {
    //Locate the king
    int otherKing = -1;
    for (int i = 0; i < 64; i++) {
        if ((isWhiteCheck && pieceAt(i, boardLayout) == WhiteKing) || (!isWhiteCheck && pieceAt(i, boardLayout) == BlackKing)) {
            otherKing = i;
            break;
        }
    }

    //If we can't locate the other king, either something has gone horribly wrong
    //or we're just checking before a board move
    if (otherKing == -1) return false;

    //See if any piece can take the king
    for (int i = 0; i < 64; i++) {
        Piece piece = pieceAt(i, boardLayout);
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
    if (!isWhiteTurn && isBlackPiece(piece)) return true;
    return false;
}

bool GameEngine::isWhitePiece(GameEngine::Piece piece) {
    return piece <= WhitePiecesEnd && piece != Empty;
}

bool GameEngine::isBlackPiece(GameEngine::Piece piece) {
    return !isWhitePiece(piece) && piece != Empty;
}

GameEngine::Piece GameEngine::pieceAt(int index) {
    return pieceAt(index, d->boardLayout);
}

GameEngine::Piece GameEngine::pieceAt(int index, QList<quint8> boardLayout) {
    return static_cast<Piece>(boardLayout.at(index));
}

QString GameEngine::pieceName(GameEngine::Piece piece) {
    QMap<Piece, const char*> pieces = {
        {Empty, QT_TR_NOOP("Empty Space")},
        {WhitePawn, QT_TR_NOOP("White Pawn")},
        {WhiteKnight, QT_TR_NOOP("White Knight")},
        {WhiteBishop, QT_TR_NOOP("White Bishop")},
        {WhiteRook, QT_TR_NOOP("White Rook")},
        {WhiteQueen, QT_TR_NOOP("White Queen")},
        {WhiteKing, QT_TR_NOOP("White King")},
        {BlackPawn, QT_TR_NOOP("Black Pawn")},
        {BlackKnight, QT_TR_NOOP("Black Knight")},
        {BlackBishop, QT_TR_NOOP("Black Bishop")},
        {BlackRook, QT_TR_NOOP("Black Rook")},
        {BlackQueen, QT_TR_NOOP("Black Queen")},
        {BlackKing, QT_TR_NOOP("Black King")},
    };
    return tr(pieces.value(piece));
}

void GameEngine::setFixedGameState(int turn) {
    d->currentBoardLayout = d->boardLayout;
    d->currentTakenPieces = d->takenPieces;

    MoveResults results = d->previousMoves.at(turn);
    d->boardLayout = results.newBoardLayout;
}

void GameEngine::restoreFixedGameState() {
    d->boardLayout = d->currentBoardLayout;
    d->takenPieces = d->currentTakenPieces;
}

QString GameEngine::turnDescription(bool isWhiteMove, GameEngine::MoveResults results) {
    switch (results.moveType) {
        case GameEngine::StandardMove:
        case GameEngine::EnPassant:
            if (results.takenPiece == Empty) {
                return tr("%1 moves %2 -> %3").arg(pieceName(results.movedPiece), indexToCoordinate(results.from), indexToCoordinate(results.to));
            } else {
                return tr("%1 moves from %2 -> %3 capturing a %4").arg(pieceName(results.movedPiece), indexToCoordinate(results.from), indexToCoordinate(results.to), pieceName(results.takenPiece));
            }
            break;
        case GameEngine::KingsideCastle:
            if (isWhiteMove) {
                return tr("White castles kingside");
            } else {
                return tr("Black castles kingside");
            }
        case GameEngine::QueensideCastle:
            if (isWhiteMove) {
                return tr("White castles queenside");
            } else {
                return tr("Black castles queenside");
            }
        case GameEngine::InitialBoardState:
            return tr("Beginning of game");
    }

    return "A turn";
}
