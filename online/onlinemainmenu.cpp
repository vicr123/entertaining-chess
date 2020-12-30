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
#include "onlinemainmenu.h"
#include "ui_onlinemainmenu.h"

#include <tvariantanimation.h>
#include "online/onlinecontroller.h"
#include <online/friendsdialog.h>
#include <online/accountdialog.h>
#include "createprivategamescreen.h"

OnlineMainMenu::OnlineMainMenu(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::OnlineMainMenu) {
    ui->setupUi(this);

    ui->spacer1->changeSize(0, SC_DPI(200), QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->topBarrier->setBounceWidget(ui->createPrivateButton);
    ui->bottomBarrier->setBounceWidget(ui->mainMenuButton);
    ui->mainMenuButton->setProperty("type", "destructive");

    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Select"));
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonB, tr("Main Menu"));

    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonA, GamepadHud::standardAction(GamepadHud::SelectAction));
    ui->gamepadHud->setButtonAction(QGamepadManager::ButtonB, [ = ] {
        ui->mainMenuButton->click();
    });
}

OnlineMainMenu::~OnlineMainMenu() {
    delete ui;
}

void OnlineMainMenu::on_mainMenuButton_clicked() {
    OnlineController::instance()->disconnectFromOnline();
}

void OnlineMainMenu::on_friendsRelationsButton_clicked() {
    FriendsDialog* d = new FriendsDialog(this);
    connect(d, &FriendsDialog::done, d, &FriendsDialog::deleteLater);
}

void OnlineMainMenu::on_accountButton_clicked() {
    AccountDialog* d = new AccountDialog(this);
    connect(d, &AccountDialog::done, d, &AccountDialog::deleteLater);
}

void OnlineMainMenu::on_createPrivateButton_clicked() {
    CreatePrivateGameScreen* screen = new CreatePrivateGameScreen(this);
    connect(screen, &CreatePrivateGameScreen::done, screen, &CreatePrivateGameScreen::deleteLater);
}

void OnlineMainMenu::on_joinPrivateButton_clicked() {

}
