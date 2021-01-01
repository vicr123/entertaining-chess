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
#ifndef BACKGROUNDCONTROLLER_H
#define BACKGROUNDCONTROLLER_H

#include <QObject>

struct BackgroundControllerPrivate;
class BackgroundController : public QObject {
        Q_OBJECT
    public:
        explicit BackgroundController(QObject* parent = nullptr);
        ~BackgroundController();

        static BackgroundController* instance();

        void install(QWidget* widget);
        void setMainWindow(QWidget* mainWindow);

    signals:

    private:
        BackgroundControllerPrivate* d;

        void forceUpdate();

        // QObject interface
    public:
        bool eventFilter(QObject* watched, QEvent* event);
};

#endif // BACKGROUNDCONTROLLER_H
