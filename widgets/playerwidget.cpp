/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2021 Victor Tran
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
#include "playerwidget.h"
#include "ui_playerwidget.h"

#include <QPainter>
#include <tvariantanimation.h>

struct PlayerWidgetPrivate {
    QString playerName;
    QColor playerCol;

    tVariantAnimation colHeight;

    bool isActive = false;
};

PlayerWidget::PlayerWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::PlayerWidget) {
    ui->setupUi(this);

    d = new PlayerWidgetPrivate();
    d->playerCol = QColor(0, 150, 255);

    d->colHeight.setStartValue(SC_DPI(3));
    d->colHeight.setEndValue(this->height());
    d->colHeight.setDuration(250);
    d->colHeight.setEasingCurve(QEasingCurve::Linear);
    connect(&d->colHeight, &tVariantAnimation::valueChanged, this, [ = ] {
        this->update();
    });
}

PlayerWidget::~PlayerWidget() {
    delete ui;
}

void PlayerWidget::setPlayerDetails(QImage image, QString name) {
    ui->nameLabel->setText(name);
    ui->picLabel->setPixmap(QPixmap::fromImage(image).scaled(SC_DPI_T(QSize(32, 32), QSize), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void PlayerWidget::setIsActive(bool isActive) {
    d->isActive = isActive;
    if (isActive) {
        d->colHeight.setDirection(tVariantAnimation::Forward);
    } else {
        d->colHeight.setDirection(tVariantAnimation::Backward);
    }
    d->colHeight.start();
}

void PlayerWidget::resizeEvent(QResizeEvent* event) {
    d->colHeight.setEndValue(this->height());
}

void PlayerWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setPen(Qt::transparent);

    int colHeight = d->colHeight.currentValue().toInt();
    painter.setBrush(d->playerCol);
    painter.drawRect(0, 0, this->width(), colHeight);

    //Draw the timeout bar
    int timeoutBarHeight = static_cast<int>(colHeight);
    painter.setBrush(d->playerCol.darker(175));
    painter.drawRect(0, 0, this->width(), timeoutBarHeight);
}
