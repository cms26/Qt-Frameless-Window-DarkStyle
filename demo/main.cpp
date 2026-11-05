// MIT License, see LICENSE. Based on Qt-Frameless-Window-DarkStyle by Juergen
// Skrotzky (https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle).

// Demo: a QMainWindow full of widgets, inside a FramelessWindow, with
// ModernStyle following the system's light/dark setting. The toolbar's
// "Theme" box switches between System, Light and Dark.
//
// Command line (also used to make the README screenshots):
//   --theme system|light|dark     start with this theme
//   --screenshot <file.png>       save a picture of the window and quit

#include "framelesswindow.h"
#include "mainwindow.h"
#include "modernstyle.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Qt Frameless Window"));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption({ "theme", "Start with this theme: system, light or dark.", "theme", "system" });
    parser.addOption({ "screenshot", "Save a picture of the window to <file> and quit.", "file" });
    parser.addOption({ "tab", "Show this page of the tab widget (0, 1, ...).", "index" });
    parser.process(app);

    ModernStyle::Theme theme = ModernStyle::Theme::System;
    if (parser.value("theme") == QLatin1String("light"))
        theme = ModernStyle::Theme::Light;
    else if (parser.value("theme") == QLatin1String("dark"))
        theme = ModernStyle::Theme::Dark;

    // The style: the application takes ownership
    auto *style = new ModernStyle(theme);
    style->setComboPopupMaxHeight(240);       // longer drop-down lists scroll
    QApplication::setStyle(style);

    // The frameless window with our main window inside
    FramelessWindow window;
    window.setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_DesktopIcon));
    auto *mainWindow = new MainWindow(style);
    if (parser.isSet("tab"))
        mainWindow->showTab(parser.value("tab").toInt());
    window.setContent(mainWindow);
    window.resize(1100, 680);
    window.show();

    if (parser.isSet("screenshot")) {
        const QString file = parser.value("screenshot");
        QTimer::singleShot(300, &window, [&window, file] {
            window.grab().save(file);
            QApplication::quit();
        });
    }
    return app.exec();
}
