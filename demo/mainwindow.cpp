// MIT License, see LICENSE. Based on Qt-Frameless-Window-DarkStyle by Juergen
// Skrotzky (https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle).

#include "mainwindow.h"

#include "modernstyle.h"
#include "ui_mainwindow.h"

#ifdef DEMO_WITH_QWT
#include "qwtmodernstyle.h"
#include <qwt_legend.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_text.h>
#include <QtMath>
#endif

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

#ifdef DEMO_WITH_QWT
    // A Qwt plot with a few curves, styled by qwtmodernstyle.h
    auto *plot = new QwtPlot(QwtText(tr("Signals")), this);
    plot->setAxisTitle(QwtAxis::XBottom, tr("Time [s]"));
    plot->setAxisTitle(QwtAxis::YLeft, tr("Amplitude"));
    plot->insertLegend(new QwtLegend, QwtPlot::BottomLegend);
    const struct { const char *name; double (*f)(double); } signals_[] = {
        { "sin", [](double t) { return qSin(t); } },
        { "cos", [](double t) { return 0.8 * qCos(1.5 * t); } },
        { "damped", [](double t) { return 1.2 * qExp(-0.15 * t) * qSin(3 * t); } },
    };
    for (const auto &signal : signals_) {
        QVector<QPointF> points;
        for (int i = 0; i <= 400; ++i) {
            const double t = i * 10.0 / 400;
            points.append({ t, signal.f(t) });
        }
        auto *curve = new QwtPlotCurve(QString::fromLatin1(signal.name));
        curve->setSamples(points);
        curve->attach(plot);
    }
    QwtModernStyle::follow(plot, style);

    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->addWidget(plot);
    ui->tabWidget->addTab(page, tr("Plot"));
#endif
}

void MainWindow::showTab(int index)
{
    ui->tabWidget->setCurrentIndex(index);
}

MainWindow::~MainWindow()
{
    delete ui;
}
