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
#include "controllers/backgroundcontroller.h"
#include <pauseoverlay.h>
#include "online/onlinecontroller.h"
#include <online/reportcontroller.h>
#include <the-libs_global.h>

#include <QUrl>
#include <musicengine.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    BackgroundController::instance()->setMainWindow(this);

    ui->setupUi(this);

    ui->menubar->setVisible(false);

    OnlineController::instance()->setMainWindow(this);

    connect(ui->mainScreen, &MainScreen::startGame, this, [ = ](GameEnginePtr engine) {
        QSize imageSize = SC_DPI_T(QSize(32, 32), QSize);
        ui->gameScreen->setPlayers(QIcon(":/assets/white-icon.svg").pixmap(imageSize).toImage(), tr("White"), QIcon(":/assets/black-icon.svg").pixmap(imageSize).toImage(), tr("Black"));
        ui->gameScreen->setGameEngine(engine);
        ui->stackedWidget->setCurrentWidget(ui->gameScreen);
    });
    connect(ui->mainScreen, &MainScreen::goToSettings, this, [ = ] {
        ui->stackedWidget->setCurrentWidget(ui->settingsPage);
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

    connect(ui->settingsPage, &SettingsScreen::goBack, this, [ = ] {
        ui->stackedWidget->setCurrentWidget(ui->mainScreen);
    });

    connect(OnlineController::instance(), &OnlineController::isOnlineChanged, this, [ = ](bool isOnline) {
        ReportController::setAutomaticReportingEnabled(this, isOnline);
    });

    PauseOverlay::registerOverlayForWindow(this, ui->centralwidget);

    MusicEngine::playBackgroundMusic();
    on_stackedWidget_currentChanged(0);

    ui->mainScreen->setFocus();
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::on_stackedWidget_currentChanged(int arg1) {
    if (ui->stackedWidget->currentWidget() == ui->gameScreen) {
        MusicEngine::setBackgroundMusic("bleeping-intro", "bleeping-loop");
    } else {
        MusicEngine::setBackgroundMusic("neon-intro", "neon-loop");
    }
}
