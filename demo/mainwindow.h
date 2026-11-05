// MIT License, see LICENSE. Based on Qt-Frameless-Window-DarkStyle by Juergen
// Skrotzky (https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle).

#pragma once

#include <QMainWindow>

class ModernStyle;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // style: the application's ModernStyle, for the theme switcher (may be null)
    explicit MainWindow(ModernStyle *style = nullptr, QWidget *parent = nullptr);
    ~MainWindow() override;

    void showTab(int index);   // in the tab widget (for --tab)

private:
    Ui::MainWindow *ui;
};
