// MIT License, see LICENSE. Based on Qt-Frameless-Window-DarkStyle by Juergen
// Skrotzky (https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle).

#include "mainwindow.h"

#include "modernstyle.h"
#include "ui_mainwindow.h"

#include <QComboBox>
#include <QLabel>
#include <QToolBar>

MainWindow::MainWindow(ModernStyle *style, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("Qt Frameless Window - Modern Style"));

    // Theme switcher in the toolbar
    if (style) {
        auto *themes = new QComboBox(this);
        themes->addItem(tr("System"), QVariant::fromValue(ModernStyle::Theme::System));
        themes->addItem(tr("Light"), QVariant::fromValue(ModernStyle::Theme::Light));
        themes->addItem(tr("Dark"), QVariant::fromValue(ModernStyle::Theme::Dark));
        themes->setCurrentIndex(themes->findData(QVariant::fromValue(style->theme())));
        connect(themes, &QComboBox::currentIndexChanged, this, [style, themes] {
            style->setTheme(themes->currentData().value<ModernStyle::Theme>());
        });

        ui->mainToolBar->addSeparator();
        ui->mainToolBar->addWidget(new QLabel(tr("  Theme: "), this));
        ui->mainToolBar->addWidget(themes);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
