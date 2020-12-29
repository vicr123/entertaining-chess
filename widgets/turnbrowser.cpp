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
#include "turnbrowser.h"
#include "ui_turnbrowser.h"

#include "game/gameengine.h"
#include <tvariantanimation.h>
#include <QPainter>
#include <QKeyEvent>

struct TurnBrowserPrivate {
    GameEngine* engine;
    QStringList entries;
    int turn;

    tVariantAnimation* turnAnim;
};

TurnBrowser::TurnBrowser(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::TurnBrowser) {
    ui->setupUi(this);

    d = new TurnBrowserPrivate();
    this->setFocusPolicy(Qt::StrongFocus);

    d->turnAnim = new tVariantAnimation(this);
    d->turnAnim->setDuration(200);
    d->turnAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(d->turnAnim, &tVariantAnimation::valueChanged, this, [ = ] {
        this->update();
    });
}

TurnBrowser::~TurnBrowser() {
    delete d;
    delete ui;
}

void TurnBrowser::setEngine(GameEngine* engine) {
    d->engine = engine;

    bool isWhiteTurn = true;
    const QStack<GameEngine::MoveResults> previousMoves = engine->previousMoves();
    for (const GameEngine::MoveResults& results : previousMoves) {
        QString description = engine->turnDescription(isWhiteTurn, results);
        d->entries.append(description);
        isWhiteTurn = !isWhiteTurn;
    }
    d->turn = d->entries.count() - 1;
    d->turnAnim->setStartValue(static_cast<qreal>(d->turn));
    d->turnAnim->setEndValue(static_cast<qreal>(d->turn));
}

void TurnBrowser::nextTurn() {
    int newTurn = d->turn + 1;
    if (newTurn >= d->entries.count()) newTurn = d->entries.count() - 1;
    d->turn = newTurn;
    emit currentTurnChanged(newTurn);

    d->turnAnim->stop();
    d->turnAnim->setStartValue(d->turnAnim->currentValue().toReal());
    d->turnAnim->setEndValue(static_cast<qreal>(newTurn));
    d->turnAnim->start();
}

void TurnBrowser::previousTurn() {
    int newTurn = d->turn - 1;
    if (newTurn < 0) newTurn = 0;
    d->turn = newTurn;
    emit currentTurnChanged(newTurn);

    d->turnAnim->stop();
    d->turnAnim->setStartValue(d->turnAnim->currentValue().toReal());
    d->turnAnim->setEndValue(static_cast<qreal>(newTurn));
    d->turnAnim->start();
}

void TurnBrowser::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    //Draw the arrows
    QRect arrowRect;
    arrowRect.setSize(SC_DPI_T(QSize(16, 16), QSize));
    arrowRect.moveLeft(this->width() / 2 - arrowRect.width() / 2);
    arrowRect.moveTop(0);

    QColor transparentCol = this->palette().color(QPalette::WindowText);
    transparentCol.setAlpha(0);

    QLinearGradient grad;
    grad.setStart(0, 0);
    grad.setFinalStop(0, this->height());
    grad.setColorAt(0, transparentCol);
    grad.setColorAt(0.3, this->palette().color(QPalette::WindowText));
    grad.setColorAt(0.7, this->palette().color(QPalette::WindowText));
    grad.setColorAt(1, transparentCol);
    painter.setPen(QPen(grad, 1));

    QFont font = this->font();
    font.setPixelSize(SC_DPI(20));
    painter.setFont(font);

    QRect primaryTextRect;
    primaryTextRect.setWidth(this->width());
    primaryTextRect.setHeight(painter.fontMetrics().height() + SC_DPI(5));
    primaryTextRect.moveCenter(QPoint(this->width() / 2, this->height() / 2));

    primaryTextRect.moveTop(primaryTextRect.top() - primaryTextRect.height() * d->turnAnim->currentValue().toReal());

    for (const QString& entry : qAsConst(d->entries)) {
        painter.drawText(primaryTextRect, Qt::AlignCenter, entry);
        primaryTextRect.moveTop(primaryTextRect.top() + primaryTextRect.height());
    }

    painter.drawPixmap(arrowRect, QIcon::fromTheme("arrow-up").pixmap(arrowRect.size()));
    arrowRect.moveBottom(this->height());
    painter.drawPixmap(arrowRect, QIcon::fromTheme("arrow-down").pixmap(arrowRect.size()));
}

QSize TurnBrowser::sizeHint() const {
    return SC_DPI_T(QSize(300, 70), QSize);
}


bool TurnBrowser::event(QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Down) {
            nextTurn();
        } else if (keyEvent->key() == Qt::Key_Up) {
            previousTurn();
        }
    }
    return QWidget::event(event);
}


void TurnBrowser::resizeEvent(QResizeEvent* event) {
    QRect focusDecorationGeometry;
    focusDecorationGeometry.setHeight(SC_DPI(25));
    focusDecorationGeometry.setWidth(this->width());
    focusDecorationGeometry.moveCenter(QPoint(this->width() / 2, this->height() / 2));
    this->setProperty("X-Contemporary-FocusDecorationGeometry", focusDecorationGeometry);
}
