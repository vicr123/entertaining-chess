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

#include <QDir>
#include <tapplication.h>
#include <notificationengine.h>
#include <entertaining.h>

int main(int argc, char* argv[]) {
    tApplication a(argc, argv);
    a.registerCrashTrap();

    if (QDir("/usr/share/entertaining-chess").exists()) {
        a.setShareDir("/usr/share/entertaining-chess");
    } else if (QDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/entertaining-chess/")).exists()) {
        a.setShareDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/entertaining-chess/"));
    }
    a.installTranslators();

    a.setOrganizationName("theSuite");
    a.setOrganizationDomain("");
    a.setApplicationIcon(QIcon::fromTheme("entertaining-chess", QIcon(":/icons/entertaining-chess.svg")));
    a.setApplicationVersion("0.1");
    a.setGenericName(QApplication::translate("main", "Chess"));
//    a.setAboutDialogSplashGraphic(a.aboutDialogSplashGraphicFromSvg(":/icons/aboutsplash.svg"));
    a.setApplicationLicense(tApplication::Gpl3OrLater);
    a.setCopyrightHolder("Victor Tran");
    a.setCopyrightYear("2019");
    a.setDesktopFileName("com.vicr123.entertaining.chess");
#ifdef T_BLUEPRINT_BUILD
    a.setApplicationName("Entertaining Chess Blueprint");
#else
    a.setApplicationName("Entertaining Chess");
#endif

    Entertaining::initialize();

    MainWindow w;
    w.show();

    NotificationEngine::setApplicationNotificationWindow(&w);

    return a.exec();
}
