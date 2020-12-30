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
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "online/onlinecontroller.h"
#include <pauseoverlay.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->menubar->setVisible(false);

    OnlineController::instance()->setMainWindow(this);

    connect(ui->mainScreen, &MainScreen::startGame, this, [ = ](GameEngine * engine) {
        ui->gameScreen->setGameEngine(engine);
        ui->stackedWidget->setCurrentWidget(ui->gameScreen);
    });

    connect(OnlineController::instance(), &OnlineController::isOnlineChanged, this, [ = ](bool isOnline) {
        if (isOnline) {
            ui->stackedWidget->setCurrentWidget(ui->onlineScreen);
        } else {
            ui->stackedWidget->setCurrentWidget(ui->mainScreen);
        }
    });

    connect(ui->gameScreen, &GameScreen::returnToMainMenu, this, [ = ] {
        ui->stackedWidget->setCurrentWidget(ui->mainScreen);
    });

    PauseOverlay::registerOverlayForWindow(this, ui->centralwidget);
}

MainWindow::~MainWindow() {
    delete ui;
}

