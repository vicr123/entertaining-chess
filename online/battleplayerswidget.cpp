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
#include "battleplayerswidget.h"
#include "ui_battleplayerswidget.h"

#include <QIcon>
#include <tvariantanimation.h>

BattlePlayersWidget::BattlePlayersWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::BattlePlayersWidget) {
    ui->setupUi(this);

    ui->mainLayout->setHorizontalSpacing(SC_DPI(20));

    ui->crossedSwordsIcon->setPixmap(QIcon(":/assets/crossed-swords.svg").pixmap(SC_DPI_T(QSize(32, 32), QSize)));
    clearWhiteSide();
    clearBlackSide();
}

BattlePlayersWidget::~BattlePlayersWidget() {
    delete ui;
}

void BattlePlayersWidget::setWhiteSide(QImage image, QString name) {
    ui->whitePicture->setPixmap(QPixmap::fromImage(image).scaled(SC_DPI_T(QSize(64, 64), QSize)));
    ui->whiteName->setText(name);
}

void BattlePlayersWidget::clearWhiteSide() {
    ui->whitePicture->setPixmap(QIcon(":/assets/open-space.svg").pixmap(SC_DPI_T(QSize(64, 64), QSize)));
    ui->whiteName->setText(tr("Nobody"));
}

void BattlePlayersWidget::setBlackSide(QImage image, QString name) {
    ui->blackPicture->setPixmap(QPixmap::fromImage(image).scaled(SC_DPI_T(QSize(64, 64), QSize)));
    ui->blackName->setText(name);
}

void BattlePlayersWidget::clearBlackSide() {
    ui->blackPicture->setPixmap(QIcon(":/assets/open-space.svg").pixmap(SC_DPI_T(QSize(64, 64), QSize)));
    ui->blackName->setText(tr("Nobody"));
}
