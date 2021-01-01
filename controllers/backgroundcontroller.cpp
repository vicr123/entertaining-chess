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
#include "backgroundcontroller.h"

#include <tvariantanimation.h>
#include <QWidget>
#include <QEvent>
#include <QPainter>
#include <QTimer>

struct BackgroundControllerPrivate {
    QPixmap backgroundImage;
    QList<QWidget*> widgets;

    QWidget* mainWindow;
    tVariantAnimation* stageProgress;
    int stage = 0;
    bool backgroundBlack = true;
};

BackgroundController::BackgroundController(QObject* parent) : QObject(parent) {
    d = new BackgroundControllerPrivate();

    d->stageProgress = new tVariantAnimation();
    d->stageProgress->setStartValue(0.0);
    d->stageProgress->setEndValue(1.0);
    d->stageProgress->setEasingCurve(QEasingCurve::Linear);
    d->stageProgress->setDuration(20000);
    connect(d->stageProgress, &tVariantAnimation::valueChanged, this, &BackgroundController::forceUpdate);
    connect(d->stageProgress, &tVariantAnimation::finished, this, [ = ] {
        d->stage++;
        if (d->stage == 2) d->stage = 0;

        updateImage();

        d->stageProgress->start();
    });

    QTimer::singleShot(1000, this, [ = ] {
        d->stageProgress->start();
        QTimer::singleShot(1000, this, [ = ] {
            d->backgroundBlack = false;
        });
    });
}

BackgroundController::~BackgroundController() {
    delete d;
}

BackgroundController* BackgroundController::instance() {
    static BackgroundController* instance = new BackgroundController();
    return instance;
}

void BackgroundController::install(QWidget* widget) {
    d->widgets.append(widget);
    widget->installEventFilter(this);
}

void BackgroundController::setMainWindow(QWidget* mainWindow) {
    d->mainWindow = mainWindow;
    mainWindow->installEventFilter(this);
}

void BackgroundController::forceUpdate() {
    for (QWidget* w : qAsConst(d->widgets)) {
        w->update();
    }
}

void BackgroundController::updateImage() {
    QString filename;
    switch (d->stage) {
        case 0:
            filename = ":/assets/background.png";
            break;
        case 1:
            filename = ":/assets/background1.png";
            break;
    }

    QPixmap image(filename);

    QSize size = image.size();
    QSize targetSize = d->mainWindow->size();
    targetSize.setWidth(targetSize.width() + SC_DPI(300));
    size.scale(targetSize, Qt::KeepAspectRatioByExpanding);

    d->backgroundImage = image.scaled(size);
}

bool BackgroundController::eventFilter(QObject* watched, QEvent* event) {
    if (watched == d->mainWindow) {
        if (event->type() == QEvent::Resize) {
            updateImage();
        }
    } else {
        if (event->type() == QEvent::Paint) {
            QWidget* widget = static_cast<QWidget*>(watched);

            QPainter painter(widget);
            painter.fillRect(0, 0, widget->width(), widget->height(), d->backgroundBlack ? Qt::black : QColor(40, 40, 40));

            qreal opacity;
            if (d->stageProgress->currentTime() < 1000) {
                opacity = d->stageProgress->currentTime() / 1000.0;
            } else if (d->stageProgress->currentTime() > 19000) {
                opacity = 1 - (d->stageProgress->currentTime() - 19000) / 1000.0;
            } else {
                opacity = 1;
            }
            painter.setOpacity(opacity);

            painter.fillRect(0, 0, widget->width(), widget->height(), QColor(40, 40, 40));

            QPoint center = QPoint(widget->width() / 2, widget->height() / 2);
            center.rx() -= (SC_DPI(300) * d->stageProgress->currentValue().toReal()) - SC_DPI(150);

            QRect geometry;
            geometry.setSize(d->backgroundImage.size());
            geometry.moveCenter(center);
            geometry.moveTop(geometry.top());
            painter.drawPixmap(geometry, d->backgroundImage);

            QLinearGradient grad(QPoint(0, widget->height()), QPoint(0, widget->height() - SC_DPI(50)));
            grad.setColorAt(0, QColor(0, 0, 0, 127));
            grad.setColorAt(1, QColor(0, 0, 0, 0));

            painter.setBrush(grad);
            painter.setPen(Qt::transparent);
            painter.drawRect(0, 0, widget->width(), widget->height());
        }
    }
    return false;
}
