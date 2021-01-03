#ifndef SETTINGSSCREEN_H
#define SETTINGSSCREEN_H

#include <QWidget>

namespace Ui {
    class SettingsScreen;
}

struct SettingsScreenPrivate;
class SettingsScreen : public QWidget {
        Q_OBJECT

    public:
        explicit SettingsScreen(QWidget* parent = nullptr);
        ~SettingsScreen();

        void updateSettings();

    signals:
        void goBack();

    private slots:
        void on_titleLabel_backButtonClicked();

    private:
        Ui::SettingsScreen* ui;
        SettingsScreenPrivate* d;

        void resizeEvent(QResizeEvent* event);
};

#endif // SETTINGSSCREEN_H
