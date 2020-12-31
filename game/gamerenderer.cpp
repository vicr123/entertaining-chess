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
#include "gamerenderer.h"

#include "gameengine.h"
#include <QMap>
#include <QPainter>
#include <QSvgRenderer>
#include <QMouseEvent>
#include <tvariantanimation.h>
#include "screens/promotescreen.h"
#include <pauseoverlay.h>
#include <terrorflash.h>

struct AnimatedPiece {
    QPointF currentLocation;
    GameEngine::Piece piece;
};

struct GameRendererPrivate {
    GameEnginePtr gameEngine;

    int currentFocus = -1;
    int currentSelection = -1;
    QList<int> availableMoves;

    int fixedGameStateTurn = -1;

    tVariantAnimation* boardRotation;

    QList<AnimatedPiece*> animations;
    QList<int> animationsTo;
};

GameRenderer::GameRenderer(QWidget* parent) : QWidget(parent) {
    d = new GameRendererPrivate();

    this->setMouseTracking(true);

    d->boardRotation = new tVariantAnimation(this);
    d->boardRotation->setDuration(1000);
    d->boardRotation->setEasingCurve(QEasingCurve::InOutCubic);
    connect(d->boardRotation, &tVariantAnimation::valueChanged, this, [ = ] {
        this->update();
    });
    d->boardRotation->start();
}

GameRenderer::~GameRenderer() {
    delete d;
}

void GameRenderer::setGameEngine(GameEnginePtr engine) {
    if (d->gameEngine != nullptr) d->gameEngine->disconnect(this);

    d->currentFocus = -1;
    d->currentSelection = -1;
    d->availableMoves.clear();

    d->gameEngine = engine;
    connect(engine.data(), &GameEngine::moveIssued, this, [ = ] {
        this->update();
        updateBoardRotation();
    });
    connect(engine.data(), &GameEngine::animatePiece, this, [ = ](int from, int to, GameEngine::Piece piece) {
        AnimatedPiece* ap = new AnimatedPiece();
        ap->piece = piece;

        d->animations.append(ap);
        d->animationsTo.append(to);

        tVariantAnimation* anim = new tVariantAnimation();
        anim->setStartValue(QPointF(from % 8, from / 8));
        anim->setEndValue(QPointF(to % 8, to / 8));
        anim->setDuration(500);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, &tVariantAnimation::valueChanged, this, [ = ](QVariant value) {
            ap->currentLocation = value.toPointF();
            this->update();
        });
        connect(anim, &tVariantAnimation::finished, this, [ = ] {
            d->animationsTo.removeOne(to);
            d->animations.removeOne(ap);
            delete ap;

            updateBoardRotation();
        });
        anim->start();
    });

    int boardRotation = this->shouldRotateToWhite() ? 0 : 180;
    d->boardRotation->setStartValue(boardRotation);
    d->boardRotation->setEndValue(boardRotation);
    this->update();
}

GameEnginePtr GameRenderer::gameEngine() {
    return d->gameEngine;
}

void GameRenderer::setFixedGameState(int turn) {
    d->fixedGameStateTurn = turn;
    this->update();
}

QRect GameRenderer::viewport() {
    QRect viewport(0, 0, 1, 1);
    viewport.setSize(viewport.size().scaled(this->size(), Qt::KeepAspectRatio));
    viewport.moveCenter(QPoint(this->width() / 2, this->height() / 2));
    return viewport;
}

void GameRenderer::select() {
    if (d->gameEngine->isHumanTurn() && d->fixedGameStateTurn == -1) {
        if (d->currentSelection != -1) {
            if (d->currentSelection == d->currentFocus) {
                //Deselect the piece
                d->currentSelection = -1;
                d->availableMoves.clear();
            } else if (d->availableMoves.contains(d->currentFocus)) {
                //Perform the move
                if ((d->gameEngine->pieceAt(d->currentSelection) == GameEngine::WhitePawn && d->currentFocus <= 7) || (d->gameEngine->pieceAt(d->currentSelection) == GameEngine::BlackPawn && d->currentFocus >= 56)) {
                    //White promotion!
                    PromoteScreen* promoteScreen = new PromoteScreen(d->gameEngine->isWhiteTurn());
                    connect(promoteScreen, &PromoteScreen::dismiss, this, [ = ] {
                        PauseOverlay::overlayForWindow(this)->popOverlayWidget();
                    });
                    connect(promoteScreen, &PromoteScreen::promote, this, [ = ](GameEngine::Piece piece) {
                        PauseOverlay::overlayForWindow(this)->popOverlayWidget();
                        d->gameEngine->issueMove(d->currentSelection, d->currentFocus, piece);
                        d->currentSelection = -1;
                        d->availableMoves.clear();
                    });
                    PauseOverlay::overlayForWindow(this)->pushOverlayWidget(promoteScreen);
                } else {
                    d->gameEngine->issueMove(d->currentSelection, d->currentFocus);
                    d->currentSelection = -1;
                    d->availableMoves.clear();
                }
            }
        } else if (d->currentFocus != -1) {
            //Find valid moves
            d->currentSelection = d->currentFocus;
            for (int i = 0; i < 64; i++) {
                if (d->gameEngine->isValidMove(d->currentSelection, i)) d->availableMoves.append(i);
            }
            if (d->availableMoves.isEmpty()) {
                //Invalid selection
                d->currentSelection = -1;
            }
        }
    }

    this->update();
}

void GameRenderer::updateBoardRotation() {
    //Make sure no pieces are animating
    if (!d->animationsTo.isEmpty()) return;


    int currentRotation = d->boardRotation->currentValue().toInt() % 360;
    int boardRotation = this->shouldRotateToWhite() ? 360 : 180;

    if (currentRotation == boardRotation % 360) return; //Don't need to animate anything

    d->boardRotation->setStartValue(currentRotation);
    d->boardRotation->setEndValue(boardRotation);
    d->boardRotation->start();
}

bool GameRenderer::shouldRotateToWhite() {
    if (d->gameEngine->isWhiteHuman() && d->gameEngine->isBlackHuman()) {
        return d->gameEngine->isWhiteTurn();
    } else if (d->gameEngine->isWhiteHuman()) {
        return true;
    } else {
        return false;
    }
}

void GameRenderer::paintEvent(QPaintEvent* event) {
    if (d->fixedGameStateTurn != -1) d->gameEngine->setFixedGameState(d->fixedGameStateTurn);

    QPixmap pixmap(this->width(), this->height());
    pixmap.fill(this->palette().color(QPalette::Window));

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setWindow(QRect(0, 0, 8, 8));
    painter.setViewport(this->viewport());

    bool whiteCheck = false;
    bool blackCheck = false;

    if (d->gameEngine->isCheck()) {
        if (d->gameEngine->isWhiteTurn()) {
            whiteCheck = true;
        } else {
            blackCheck = true;
        }
    }

    //Draw the chess grid
    for (int i = 0; i < 64; i++) {
        QRect rect(i % 8, i / 8, 1, 1);
        bool white = true;
        if (i / 8 % 2) white = !white;
        if (i % 2) white = !white;
        painter.fillRect(rect, white ? Qt::white : Qt::gray);
    }

    //Draw the focus decoration
    if (d->currentSelection != -1) {
        painter.fillRect(QRect(d->currentSelection % 8, d->currentSelection / 8, 1, 1), QColor(0, 200, 0));
    }
    if (d->currentFocus != -1 && d->gameEngine->isHumanTurn()) {
        painter.fillRect(QRect(d->currentFocus % 8, d->currentFocus / 8, 1, 1), QColor(0, 200, 255));
    }

    const QMap<GameEngine::Piece, QString> pieceIcons = {
        {GameEngine::WhitePawn, "white-pawn"},
        {GameEngine::WhiteKnight, "white-knight"},
        {GameEngine::WhiteBishop, "white-bishop"},
        {GameEngine::WhiteRook, "white-rook"},
        {GameEngine::WhiteQueen, "white-queen"},
        {GameEngine::WhiteKing, "white-king"},
        {GameEngine::BlackPawn, "black-pawn"},
        {GameEngine::BlackKnight, "black-knight"},
        {GameEngine::BlackBishop, "black-bishop"},
        {GameEngine::BlackRook, "black-rook"},
        {GameEngine::BlackQueen, "black-queen"},
        {GameEngine::BlackKing, "black-king"}
    };
//    const QMap<GameEngine::Piece, QString> pieceIcons = {
//        {GameEngine::WhitePawn, QChar(0x2659)},
//        {GameEngine::WhiteKnight, QChar(0x2658)},
//        {GameEngine::WhiteBishop, QChar(0x2657)},
//        {GameEngine::WhiteRook, QChar(0x2656)},
//        {GameEngine::WhiteQueen, QChar(0x2655)},
//        {GameEngine::WhiteKing, QChar(0x2654)},
//        {GameEngine::BlackPawn, QChar(0x265F)},
//        {GameEngine::BlackKnight, QChar(0x265E)},
//        {GameEngine::BlackBishop, QChar(0x265D)},
//        {GameEngine::BlackRook, QChar(0x265C)},
//        {GameEngine::BlackQueen, QChar(0x265B)},
//        {GameEngine::BlackKing, QChar(0x265A)}
//    };

    //Draw the chess pieces
    QFont font = painter.font();
    font.setPixelSize(1);
    painter.setFont(font);
    painter.setPen(Qt::black);
    for (int i = 0; i < 64; i++) {
        //Don't draw pieces we're currently animating
        if (d->animationsTo.contains(i)) continue;
        QRectF rect(i % 8, i / 8, 1, 1);

        GameEngine::Piece piece = d->gameEngine->pieceAt(i);
        if (piece == GameEngine::Empty) continue;

        QPixmap piecePx(512, 512);
        piecePx.fill(Qt::transparent);

        QPainter piecePainter(&piecePx);
        piecePainter.translate(256, 256);
        piecePainter.rotate(-d->boardRotation->currentValue().toInt());

        QSvgRenderer renderer(QStringLiteral(":/assets/%1.svg").arg(pieceIcons.value(piece)));
        renderer.render(&piecePainter, QRect(-128, -128, 256, 256));

        piecePainter.end();

        QRectF pixmapRect;
        pixmapRect.setSize(QSizeF(2, 2));
        pixmapRect.moveCenter(QPointF(i % 8 + 0.5, i / 8 + 0.5));
        painter.drawPixmap(pixmapRect, piecePx, QRectF(0, 0, 512, 512));

        if ((piece == GameEngine::WhiteKing && whiteCheck) || (piece == GameEngine::BlackKing && blackCheck)) {
            QColor checkColor(100, 0, 0);
            painter.fillRect(rect.adjusted(0, 0, 0, -0.9), checkColor);
            painter.fillRect(rect.adjusted(0, 0, -0.9, 0), checkColor);
            painter.fillRect(rect.adjusted(0.9, 0, 0, 0), checkColor);
            painter.fillRect(rect.adjusted(0, 0.9, 0, 0), checkColor);
        }
    }

    //Draw animating pieces
    for (AnimatedPiece* anim : d->animations) {
        QRectF rect(anim->currentLocation, QSizeF(1, 1));

        QPixmap piecePx(512, 512);
        piecePx.fill(Qt::transparent);

        QPainter piecePainter(&piecePx);
        piecePainter.translate(256, 256);
        piecePainter.rotate(-d->boardRotation->currentValue().toInt());

        QSvgRenderer renderer(QStringLiteral(":/assets/%1.svg").arg(pieceIcons.value(anim->piece)));
        renderer.render(&piecePainter, QRect(-128, -128, 256, 256));

        piecePainter.end();

        QRectF pixmapRect;
        pixmapRect.setSize(QSizeF(2, 2));
        pixmapRect.moveCenter(anim->currentLocation + QPointF(0.5, 0.5));
        painter.drawPixmap(pixmapRect, piecePx, QRectF(0, 0, 512, 512));
    }

    //Draw the available moves
    for (int move : d->availableMoves) {
        QRectF circleRect;
        circleRect.setWidth(0.25);
        circleRect.setHeight(0.25);
        circleRect.moveCenter(QPointF(move % 8 + 0.5, move / 8 + 0.5));
        painter.setBrush(QColor(0, 200, 0));
        painter.setPen(Qt::transparent);
        painter.drawEllipse(circleRect);
    }

    if (d->fixedGameStateTurn != -1) d->gameEngine->restoreFixedGameState();

    painter.end();

    QPainter windowPainter(this);
    windowPainter.translate(this->width() / 2, this->height() / 2);
    windowPainter.rotate(d->boardRotation->currentValue().toInt());
    windowPainter.drawPixmap(-this->width() / 2, -this->height() / 2, pixmap);
}

void GameRenderer::mousePressEvent(QMouseEvent* event) {
    select();
}

void GameRenderer::mouseReleaseEvent(QMouseEvent* event) {

}

void GameRenderer::mouseMoveEvent(QMouseEvent* event) {
    if (d->gameEngine->isHumanTurn() && d->fixedGameStateTurn == -1) {
        //Perform hit testing
        QRect viewport = this->viewport();
        if (!viewport.contains(event->pos())) {
            d->currentFocus = -1;
            this->update();
            return;
        }

        QPoint transformedPos = event->pos() - viewport.topLeft();
        int eighth = viewport.height() / 8;
        int x = transformedPos.x() / eighth;
        int y = transformedPos.y() / eighth;
        d->currentFocus = y * 8 + x;

        int boardRotation = d->boardRotation->currentValue().toInt() % 360;
        if (boardRotation > 90 && boardRotation < 270) {
            //Consider the board flipped
            d->currentFocus = 63 - d->currentFocus;
        }
        this->update();
    }
}

void GameRenderer::leaveEvent(QEvent* event) {
    d->currentFocus = -1;
    this->update();
}
