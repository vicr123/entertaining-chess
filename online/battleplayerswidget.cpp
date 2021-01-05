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

#include <QPainter>
#include <QIcon>
#include <tvariantanimation.h>

struct BattlePlayersWidgetPrivate {
    tVariantAnimation* whiteSideAnim, *blackSideAnim;
    QImage whiteSidePicture, blackSidePicture;
};

BattlePlayersWidget::BattlePlayersWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::BattlePlayersWidget) {
    ui->setupUi(this);

    d = new BattlePlayersWidgetPrivate();

    d->whiteSideAnim = new tVariantAnimation(this);
    d->whiteSideAnim->setStartValue(0.0);
    d->whiteSideAnim->setEndValue(1.0);
    d->whiteSideAnim->setDuration(500);
    connect(d->whiteSideAnim, &tVariantAnimation::valueChanged, this, [ = ] {
        this->update();
    });

    d->blackSideAnim = new tVariantAnimation(this);
    d->blackSideAnim->setStartValue(0.0);
    d->blackSideAnim->setEndValue(1.0);
    d->blackSideAnim->setDuration(500);
    connect(d->blackSideAnim, &tVariantAnimation::valueChanged, this, [ = ] {
        this->update();
    });

    ui->mainLayout->setHorizontalSpacing(SC_DPI(20));

    ui->whitePicture->setFixedSize(SC_DPI_T(QSize(64, 64), QSize));
    ui->blackPicture->setFixedSize(SC_DPI_T(QSize(64, 64), QSize));

    ui->crossedSwordsIcon->setPixmap(QIcon(":/assets/crossed-swords.svg").pixmap(SC_DPI_T(QSize(32, 32), QSize)));
    clearWhiteSide();
    clearBlackSide();
}

BattlePlayersWidget::~BattlePlayersWidget() {
    delete ui;
}

void BattlePlayersWidget::setWhiteSide(QImage image, QString name) {
    d->whiteSidePicture = image;
    ui->whiteName->setText(name);

    d->whiteSideAnim->setStartValue(d->whiteSideAnim->currentValue());
    d->whiteSideAnim->setEndValue(1.0);
    d->whiteSideAnim->setEasingCurve(QEasingCurve::OutBack);
    d->whiteSideAnim->start();
}

void BattlePlayersWidget::clearWhiteSide() {
    ui->whiteName->setText(tr("Nobody"));

    d->whiteSideAnim->setStartValue(d->whiteSideAnim->currentValue());
    d->whiteSideAnim->setEndValue(0.0);
    d->whiteSideAnim->setEasingCurve(QEasingCurve::InCubic);
    d->whiteSideAnim->start();
}

void BattlePlayersWidget::setBlackSide(QImage image, QString name) {
    d->blackSidePicture = image;
    ui->blackName->setText(name);

    d->blackSideAnim->setStartValue(d->blackSideAnim->currentValue());
    d->blackSideAnim->setEndValue(1.0);
    d->blackSideAnim->setEasingCurve(QEasingCurve::OutBack);
    d->blackSideAnim->start();
}

void BattlePlayersWidget::clearBlackSide() {
    ui->blackName->setText(tr("Nobody"));

    d->blackSideAnim->setStartValue(d->blackSideAnim->currentValue());
    d->blackSideAnim->setEndValue(0.0);
    d->blackSideAnim->setEasingCurve(QEasingCurve::InCubic);
    d->blackSideAnim->start();
}

void BattlePlayersWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPixmap openSpace = QIcon(":/assets/open-space.svg").pixmap(SC_DPI_T(QSize(64, 64), QSize));
    painter.drawPixmap(ui->whitePicture->geometry(), openSpace);
    painter.drawPixmap(ui->blackPicture->geometry(), openSpace);

    QRect whitePictureGeometry;
    whitePictureGeometry.setSize(SC_DPI_T(QSize(64, 64), QSize) * d->whiteSideAnim->currentValue().toReal());
    whitePictureGeometry.moveCenter(ui->whitePicture->geometry().center());
    painter.drawImage(whitePictureGeometry, d->whiteSidePicture.scaled(whitePictureGeometry.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    QRect blackPictureGeometry;
    blackPictureGeometry.setSize(SC_DPI_T(QSize(64, 64), QSize) * d->blackSideAnim->currentValue().toReal());
    blackPictureGeometry.moveCenter(ui->blackPicture->geometry().center());
    painter.drawImage(blackPictureGeometry, d->blackSidePicture.scaled(blackPictureGeometry.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}
