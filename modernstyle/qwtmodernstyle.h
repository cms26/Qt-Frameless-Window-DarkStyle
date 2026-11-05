// MIT License, see LICENSE.
//
// QwtModernStyle - gives Qwt plots the ModernStyle look, in the light and the
// dark theme. Optional and header-only: include it only in code that uses
// Qwt (6.2 or newer); the rest of the library doesn't need Qwt.
//
//   #include "qwtmodernstyle.h"
//
//   QwtModernStyle::follow(plot, style);   // style now, and again whenever the theme changes
//
// The style sheet (modernstyle.qss) already gives the canvas its rounded
// frame and background, and the axes and legend their text colours. This adds
// what a style sheet can't reach:
//   - curves: a colour set that reads well on light and dark backgrounds,
//     2 px antialiased lines (turn off with Option::CurveColors if your code
//     colours curves itself; seriesColor() gives the colours for your own use)
//   - grid: faint solid lines instead of grey dots (a grid is added if the
//     plot has none; turn off with Option::Grid)
//   - axes: no axis line, short outward ticks, no minor ticks
//
// Call apply() again (or use follow()) after adding curves.

#pragma once

#include "modernstyle.h"

#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>

#include <QColor>
#include <QFrame>
#include <QPen>
#include <QPointer>

namespace QwtModernStyle {

enum class Option {
    Canvas = 0x1,        // canvas without the old sunken frame (the style sheet draws a rounded one)
    Axes = 0x2,          // no axis line, short ticks
    Grid = 0x4,          // faint grid (added if missing)
    CurveColors = 0x8,   // series colours and 2 px antialiased lines for every curve
    All = 0xf,
};
Q_DECLARE_FLAGS(Options, Option)
Q_DECLARE_OPERATORS_FOR_FLAGS(Options)

// The colour of the index-th series (curve, bar, ...). Eight colours that are
// distinguishable from each other and readable on both backgrounds; brighter
// for the dark theme. The first one matches ModernStyle's default accent.
inline QColor seriesColor(int index, bool dark)
{
    static const char *const colors[] = {
        "#2f6fed", // blue
        "#ef8a2f", // orange
        "#2fa36b", // green
        "#df4f4b", // red
        "#8b5cf6", // purple
        "#0ea5b7", // teal
        "#d9569a", // pink
        "#b8952a", // ochre
    };
    const QColor color(colors[index % 8]);
    return dark ? color.lighter(125) : color;
}

// Style a plot for the light (dark = false) or the dark theme
inline void apply(QwtPlot *plot, bool dark, Options options = Option::All)
{
    if (!plot)
        return;
    const bool autoReplot = plot->autoReplot();
    plot->setAutoReplot(false);

    // Canvas: the style sheet (QwtPlotCanvas { ... }) draws the rounded
    // border and background, so drop the QFrame's sunken 3D frame
    if (options & Option::Canvas) {
        if (auto *frame = qobject_cast<QFrame *>(plot->canvas()))
            frame->setFrameStyle(QFrame::NoFrame);
    }

    // Axes: no line along the axis, short outward major ticks, no minor ticks
    if (options & Option::Axes) {
        for (int position = 0; position < QwtAxis::AxisPositions; ++position) {
            QwtScaleDraw *scaleDraw = plot->axisScaleDraw(position);
            if (!scaleDraw)
                continue;
            scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
            scaleDraw->setTickLength(QwtScaleDiv::MajorTick, 4);
            scaleDraw->setTickLength(QwtScaleDiv::MediumTick, 0);
            scaleDraw->setTickLength(QwtScaleDiv::MinorTick, 0);
            scaleDraw->setSpacing(6);
            if (QwtScaleWidget *widget = plot->axisWidget(position))
                widget->setMargin(0);
        }
    }

    // Grid: faint solid major lines, no minor lines
    if (options & Option::Grid) {
        QwtPlotGrid *grid = nullptr;
        for (QwtPlotItem *item : plot->itemList(QwtPlotItem::Rtti_PlotGrid))
            grid = static_cast<QwtPlotGrid *>(item);
        if (!grid) {
            grid = new QwtPlotGrid;
            grid->attach(plot);
        }
        QColor line = dark ? QColor(255, 255, 255) : QColor(0, 0, 0);
        line.setAlpha(dark ? 26 : 22);
        grid->setMajorPen(line, 1.0, Qt::SolidLine);
        grid->setMinorPen(QPen(Qt::NoPen));
        grid->enableXMin(false);
        grid->enableYMin(false);
        grid->setZ(0);    // behind the curves
    }

    // Curves: series colours, 2 px, smooth; the line style (dashes) is kept
    if (options & Option::CurveColors) {
        int index = 0;
        for (QwtPlotItem *item : plot->itemList(QwtPlotItem::Rtti_PlotCurve)) {
            auto *curve = static_cast<QwtPlotCurve *>(item);
            curve->setPen(seriesColor(index++, dark), 2.0, curve->pen().style());
            curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
        }
    }

    plot->setAutoReplot(autoReplot);
    plot->replot();
}

// Same, for the theme the application's ModernStyle shows right now
inline void apply(QwtPlot *plot, const ModernStyle *style, Options options = Option::All)
{
    apply(plot, style && style->isDark(), options);
}

// Style the plot now, and again whenever the style switches between light and
// dark (until the plot is deleted)
inline void follow(QwtPlot *plot, ModernStyle *style, Options options = Option::All)
{
    if (!plot || !style)
        return;
    apply(plot, style, options);
    QObject::connect(style, &ModernStyle::themeChanged, plot,
                     [plot = QPointer<QwtPlot>(plot), options](bool dark) {
                         if (plot)
                             apply(plot, dark, options);
                     });
}

} // namespace QwtModernStyle
