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
#include "promotescreen.h"
#include "ui_promotescreen.h"

struct PromoteScreenPrivate {
    bool isWhiteTurn;
};

PromoteScreen::PromoteScreen(bool isWhiteTurn, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::PromoteScreen) {
    ui->setupUi(this);
    d = new PromoteScreenPrivate();
    d->isWhiteTurn = isWhiteTurn;

    ui->titleLabel->setBackButtonShown(true);
    ui->mainWidget->setFixedWidth(SC_DPI(600));
}
PromoteScreen::~PromoteScreen() {
    delete d;
    delete ui;
}

void PromoteScreen::on_titleLabel_backButtonClicked() {
    emit dismiss();
}

void PromoteScreen::on_queenButton_clicked() {
    if (d->isWhiteTurn) {
        emit promote(GameEngine::WhiteQueen);
    } else {
        emit promote(GameEngine::BlackQueen);
    }
}

void PromoteScreen::on_rookButton_clicked() {
    if (d->isWhiteTurn) {
        emit promote(GameEngine::WhiteRook);
    } else {
        emit promote(GameEngine::BlackRook);
    }
}

void PromoteScreen::on_bishopButton_clicked() {
    if (d->isWhiteTurn) {
        emit promote(GameEngine::WhiteBishop);
    } else {
        emit promote(GameEngine::BlackBishop);
    }
}


void PromoteScreen::on_knightButton_clicked() {
    if (d->isWhiteTurn) {
        emit promote(GameEngine::WhiteKnight);
    } else {
        emit promote(GameEngine::BlackKnight);
    }
}
