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

#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QSplitter>
#include <QTreeWidget>
#include <QToolBar>

MainWindow::MainWindow(ModernStyle *style, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("Qt Frameless Window - Modern Style"));

    // A long list: the drop-down stays within the style's maximum height
    // (main.cpp) and scrolls
    for (int i = 1; i <= 40; ++i)
        ui->comboBox->addItem(tr("Item %1").arg(i));

    // The first tab: a tree and the text box side by side, with a splitter
    // between them
    auto *outline = new QTreeWidget;
    outline->setHeaderHidden(true);
    for (const QString &chapter : { tr("Introduction"), tr("Setup"), tr("Usage") }) {
        auto *item = new QTreeWidgetItem(outline, { chapter });
        for (int i = 1; i <= 3; ++i)
            new QTreeWidgetItem(item, { tr("Section %1").arg(i) });
    }
    outline->expandToDepth(0);
    auto *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(outline);
    ui->tab->layout()->replaceWidget(ui->textEdit, splitter);
    splitter->addWidget(ui->textEdit);
    splitter->setSizes({ 160, 400 });

    // Theme switcher in the toolbar, filled from the style's theme names
    if (style) {
        auto *themes = new QComboBox(this);
        themes->addItems(style->getThemeNames());
        themes->setCurrentText(ModernStyle::themeName(style->theme()));
        connect(themes, &QComboBox::currentTextChanged, this, [style](const QString &name) {
            style->setTheme(ModernStyle::themeFromName(name));
        });

        ui->mainToolBar->addSeparator();
        ui->mainToolBar->addWidget(new QLabel(tr("  Theme: "), this));
        ui->mainToolBar->addWidget(themes);
    }

    // Dock widgets: a form, a list (tabbed with the form) and a log. They
    // are panels like Slate's cards: a lighter background with a header.
    auto *properties = new QWidget;
    auto *form = new QFormLayout(properties);
    form->addRow(tr("Name"), new QLineEdit(tr("Signal 1")));
    auto *rate = new QSpinBox;
    rate->setRange(1, 10000);
    rate->setValue(250);
    rate->setSuffix(tr(" Hz"));
    form->addRow(tr("Rate"), rate);
    form->addRow(QString(), new QCheckBox(tr("Enabled")));
    auto *propertiesDock = new QDockWidget(tr("Properties"), this);
    propertiesDock->setObjectName(QStringLiteral("propertiesDock"));
    propertiesDock->setWidget(properties);
    addDockWidget(Qt::RightDockWidgetArea, propertiesDock);

    auto *layers = new QListWidget;
    layers->addItems({ tr("Background"), tr("Grid"), tr("Signals"), tr("Markers") });
    auto *layersDock = new QDockWidget(tr("Layers"), this);
    layersDock->setObjectName(QStringLiteral("layersDock"));
    layersDock->setWidget(layers);
    addDockWidget(Qt::RightDockWidgetArea, layersDock);
    tabifyDockWidget(propertiesDock, layersDock);
    propertiesDock->raise();

    auto *log = new QPlainTextEdit;
    log->setReadOnly(true);
    log->setPlainText(tr("12:00:01  Started\n12:00:02  Connected to device\n12:00:05  Recording"));
    auto *logDock = new QDockWidget(tr("Log"), this);
    logDock->setObjectName(QStringLiteral("logDock"));
    logDock->setWidget(log);
    addDockWidget(Qt::RightDockWidgetArea, logDock);

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
