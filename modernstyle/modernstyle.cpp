// MIT License, see LICENSE. Based on Qt-Frameless-Window-DarkStyle by Juergen
// Skrotzky (https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle).

#include "modernstyle.h"

#include "docktitlebar.h"

#include <QApplication>
#include <QAbstractButton>
#include <QMouseEvent>
#include <QDir>
#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QAbstractItemView>
#include <QComboBox>
#include <QDockWidget>
#include <QMainWindow>
#include <QPointer>
#include <QTabBar>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <QMetaEnum>
#include <QStyleFactory>
#include <QStyleHints>
#include <QTemporaryDir>

#include <algorithm>
#include <functional>

namespace {

// setDisabledButtonCursor(): shows Qt::ForbiddenCursor while the mouse is
// over a disabled button. Qt clears the cursor of disabled widgets and sends
// them no mouse events, so this watches the mouse moving over the
// application's windows (an application-wide event filter) and sets an
// override cursor while a disabled button is under it.
class DisabledButtonCursor : public QObject
{
public:
    explicit DisabledButtonCursor(QObject *parent)
        : QObject(parent)
    {
        qApp->installEventFilter(this);
    }

    ~DisabledButtonCursor() override
    {
        show(false);
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched->isWindowType()) {
            switch (event->type()) {
            case QEvent::MouseMove:
                show(disabledButtonAt(static_cast<QMouseEvent *>(event)->globalPosition().toPoint()));
                break;
            case QEvent::Leave:              // the mouse left the window
            case QEvent::Hide:
                show(false);
                break;
            default:
                break;
            }
        }
        return QObject::eventFilter(watched, event);
    }

private:
    static bool disabledButtonAt(const QPoint &globalPos)
    {
        for (QWidget *w = QApplication::widgetAt(globalPos); w; w = w->parentWidget()) {
            if (qobject_cast<QAbstractButton *>(w))
                return !w->isEnabled();
            if (w->isWindow())
                break;
        }
        return false;
    }

    void show(bool forbidden)
    {
        if (forbidden == m_shown)
            return;
        m_shown = forbidden;
        if (forbidden)
            QGuiApplication::setOverrideCursor(Qt::ForbiddenCursor);
        else
            QGuiApplication::restoreOverrideCursor();
    }

    bool m_shown = false;
};


// The two palettes, exactly as the application's own dark and white
// palettes set them (all roles, with the disabled ones). standardPalette()
// returns these; the style sheet's colours (themeColors()) are made from the
// same values.
QPalette themePalette(bool dark)
{
    QPalette p;
    if (dark) {
        p.setColor(QPalette::Window, QColor(53, 53, 53));
        p.setColor(QPalette::WindowText, Qt::white);
        p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(175, 175, 175));
        p.setColor(QPalette::Base, QColor(42, 42, 42));
        p.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
        p.setColor(QPalette::ToolTipBase, QColor(42, 42, 42));
        p.setColor(QPalette::ToolTipText, Qt::white);
        p.setColor(QPalette::Text, Qt::white);
        p.setColor(QPalette::Disabled, QPalette::Text, QColor(175, 175, 175));
        p.setColor(QPalette::Light, QColor(88, 88, 88));
        p.setColor(QPalette::Midlight, QColor(70, 70, 70));
        p.setColor(QPalette::Mid, QColor(44, 44, 44));
        p.setColor(QPalette::Dark, QColor(35, 35, 35));
        p.setColor(QPalette::Shadow, QColor(20, 20, 20));
        p.setColor(QPalette::Button, QColor(53, 53, 53));
        p.setColor(QPalette::ButtonText, Qt::white);
        p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(175, 175, 175));
        p.setColor(QPalette::BrightText, Qt::red);
        p.setColor(QPalette::Link, QColor(42, 130, 218));
        p.setColor(QPalette::Highlight, QColor(42, 130, 218));
        p.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
        p.setColor(QPalette::HighlightedText, Qt::white);
        p.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(175, 175, 175));
    } else {
        p.setColor(QPalette::Window, QColor(255, 255, 255));
        p.setColor(QPalette::WindowText, QColor(25, 25, 25));
        p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
        p.setColor(QPalette::Base, Qt::white);
        p.setColor(QPalette::AlternateBase, QColor(226, 226, 226));
        p.setColor(QPalette::ToolTipBase, QColor(230, 230, 230));
        p.setColor(QPalette::ToolTipText, QColor(25, 25, 25));
        p.setColor(QPalette::Text, QColor(25, 25, 25));
        p.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
        p.setColor(QPalette::Light, QColor(245, 245, 245));
        p.setColor(QPalette::Midlight, QColor(220, 220, 220));
        p.setColor(QPalette::Mid, QColor(200, 200, 200));
        p.setColor(QPalette::Dark, QColor(150, 150, 150));
        p.setColor(QPalette::Shadow, QColor(25, 25, 25));
        p.setColor(QPalette::Button, QColor(195, 195, 195));
        p.setColor(QPalette::ButtonText, QColor(25, 25, 25));
        p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
        p.setColor(QPalette::BrightText, Qt::red);
        p.setColor(QPalette::Link, QColor(17, 88, 168));
        p.setColor(QPalette::Highlight, QColor(90, 154, 209));
        p.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
        p.setColor(QPalette::HighlightedText, Qt::white);
        p.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));
    }
    return p;
}

// The style sheet's colours, @name (e.g. @border): the palette's colours,
// plus a few in-between greys for what the palette has no role for. The
// palette role each comes from is noted.
QHash<QString, QColor> themeColors(bool dark, const QColor &accent)
{
    const QPalette p = themePalette(dark);
    QHash<QString, QColor> c;
    c["window"] = p.color(QPalette::Window);
    c["surface"] = p.color(QPalette::Base);                 // text fields, lists, tables
    c["surfaceAlt"] = p.color(QPalette::AlternateBase);     // table stripes, headers
    c["text"] = p.color(QPalette::Text);
    c["textDisabled"] = p.color(QPalette::Disabled, QPalette::Text);
    c["menu"] = p.color(QPalette::Window);                  // menus: palette(window)
    c["tooltip"] = p.color(QPalette::ToolTipBase);
    if (dark) {
        c["control"] = QColor(70, 70, 70);                  // Midlight: buttons a step above the window
        c["controlHover"] = QColor(80, 80, 80);
        c["controlPressed"] = QColor(44, 44, 44);           // Mid
        c["border"] = QColor(35, 35, 35);                   // Dark
        c["borderStrong"] = QColor(88, 88, 88);             // Light
        c["groove"] = QColor(112, 112, 112);                // slider track, scroll bar handle
        c["grooveHover"] = QColor(175, 175, 175);
        c["grip"] = QColor(175, 175, 175);                  // splitter dots
        c["textSecondary"] = QColor(208, 208, 208);         // labels, headers: between text and disabled
        c["tab"] = QColor(35, 35, 35);                      // scroll bar track: Dark
        c["tabHover"] = QColor(80, 80, 80);
        c["panel"] = QColor(59, 59, 59);                    // dock widgets: AlternateBase at 50 % over Window
        c["panelHeader"] = QColor(80, 80, 80);              // their title bar
    } else {
        c["control"] = QColor(226, 226, 226);               // AlternateBase
        c["controlHover"] = QColor(208, 208, 208);
        c["controlPressed"] = QColor(195, 195, 195);        // Button
        c["border"] = QColor(200, 200, 200);                // Mid
        c["borderStrong"] = QColor(150, 150, 150);          // Dark
        c["groove"] = QColor(180, 180, 180);
        c["grooveHover"] = QColor(127, 127, 127);
        c["grip"] = QColor(127, 127, 127);
        c["textSecondary"] = QColor(74, 74, 74);
        c["tab"] = QColor(236, 236, 236);
        c["tabHover"] = QColor(220, 220, 220);              // Midlight
        c["panel"] = QColor(245, 245, 245);                 // Light
        c["panelHeader"] = QColor(226, 226, 226);           // AlternateBase
    }
    // The accent: the palette's Highlight unless the application chose one
    const QColor base = accent.isValid() ? accent : p.color(QPalette::Highlight);
    c["accent"] = base;
    c["accentHover"] = dark ? base.lighter(115) : base.darker(110);
    c["accentPressed"] = dark ? base.darker(115) : base.darker(125);
    c["accentText"] = QColor("#ffffff");      // text on the accent colour
    QColor selection = base;
    selection.setAlpha(dark ? 90 : 60);
    c["selection"] = selection;               // e.g. the frame of a checked menu icon
    return c;
}

// A vertical gradient as Bootstrap's gradient-y-three-colors() makes it:
// top colour, a middle colour reached at `position` (0..1), bottom colour
QString verticalGradient(const char *top, const char *middle, double position, const char *bottom)
{
    return QStringLiteral("qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %1, stop:%2 %3, stop:1 %4)")
        .arg(QLatin1String(top)).arg(position).arg(QLatin1String(middle)).arg(QLatin1String(bottom));
}

// Fills for buttons, tabs, bars and boxes. The style sheet refers to them as
// @name like the colours, but they can be gradients or see-through colours.
QHash<QString, QString> themeFills(bool dark, const QHash<QString, QColor> &c)
{
    QHash<QString, QString> f;
    if (dark) {
        // Buttons: raised grey gradients from Midlight (70), sunken under the
        // mouse, darker when pressed; near-black border
        const QString raised = verticalGradient("#525252", "#464646", 0.2, "#383838");
        const QString sunken = verticalGradient("#3b3b3b", "#3f3f3f", 0.4, "#434343");
        f["buttonFill"] = raised;
        f["buttonHoverFill"] = sunken;
        f["buttonPressedFill"] = verticalGradient("#2a2a2a", "#2e2e2e", 0.4, "#323232");
        f["buttonBorder"] = QStringLiteral("rgba(0, 0, 0, 153)");
        f["buttonHoverBorder"] = f["buttonBorder"];
        f["buttonText"] = QStringLiteral("#ffffff");
        // Tabs: the see-through band of the tool bars, lighter when selected
        // or under the mouse (as in the application's style sheet)
        f["tabFill"] = QStringLiteral(
            "qlineargradient(x1:0, y1:1, x2:0, y2:0, stop:0 rgba(25, 25, 25, 127), stop:1 rgba(53, 53, 53, 75))");
        f["tabHoverFill"] = QStringLiteral(
            "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(53, 53, 53, 127), stop:1 rgba(66, 66, 66, 50%))");
        f["tabSelectedFill"] = f["tabHoverFill"];
        // Tool buttons (toolbars) are flat until the mouse is over them; then
        // they light up like a raised button, a step brighter
        f["toolHoverFill"] = verticalGradient("#5e5e5e", "#525252", 0.2, "#444444");
        // Checked buttons and tool buttons: blue; darker (pressed in) when
        // the mouse is over them
        f["checkedFill"] = verticalGradient("#5a9ad1", "#428bca", 0.6, "#3681c1");
        f["checkedHoverFill"] = verticalGradient("#204c72", "#275c8a", 0.4, "#296192");
        f["checkedBorder"] = f["buttonBorder"];
        // Toolbars: a dark, see-through band with a 3 px edge towards the
        // window (top: edge at the bottom, left: edge on the right, ...)
        f["toolbarDarkEnd"] = QStringLiteral("rgba(25, 25, 25, 127)");
        f["toolbarLightEnd"] = QStringLiteral("rgba(53, 53, 53, 75)");
        f["barEdge"] = QStringLiteral("rgba(25, 25, 25, 75)");          // menu bar's bottom edge
        // Group boxes and tab pages: a lighter, see-through area
        f["boxFill"] = QStringLiteral("rgba(66, 66, 66, 50%)");
        // Menu bar item under the mouse: a grey glow fading downwards
        f["menuBarItemHoverFill"] = QStringLiteral(
            "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(106, 106, 106, 255), stop:1 rgba(106, 106, 106, 75))");
        f["menuBarItemEdge"] = QStringLiteral("rgba(106, 106, 106, 127)");
    } else {
        // Buttons: light grey gradients from AlternateBase (226), as a
        // btn-shadow(): tint 24% -> the colour at 50% -> shade 8%, border
        // shade 20%; darker under the mouse
        f["buttonFill"] = verticalGradient("#e9e9e9", "#e2e2e2", 0.5, "#d0d0d0");
        f["buttonHoverFill"] = verticalGradient("#dedede", "#d0d0d0", 0.5, "#bfbfbf");
        f["buttonPressedFill"] = verticalGradient("#bcbcbc", "#c6c6c6", 0.4, "#cdcdcd");
        f["buttonBorder"] = QStringLiteral("#b5b5b5");
        f["buttonHoverBorder"] = QStringLiteral("#a0a0a0");
        f["buttonText"] = QStringLiteral("#191919");
        // Tabs: the light band (set below); selected or under the mouse the
        // light style's replacement of the dark gradient
        f["tabHoverFill"] = QStringLiteral(
            "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(210, 210, 210, 127), stop:1 rgba(220, 220, 220, 50%))");
        f["tabSelectedFill"] = f["tabHoverFill"];
        f["toolHoverFill"] = f["buttonFill"];
        // Checked: btn-shadow() of the same blue as the dark theme (#428bca),
        // darker when hovered (btn-shadow(shade-color(#428bca, 8%)))
        // Checked: the same blue as dark; under the mouse lighter blues
        // (the light style's replacements for the dark hover blues)
        f["checkedFill"] = verticalGradient("#5a9ad1", "#428bca", 0.6, "#3681c1");
        f["checkedHoverFill"] = verticalGradient("#88b7dd", "#73a8d2", 0.4, "#5c9dd1");
        f["checkedBorder"] = QStringLiteral("#356fa2");
        // Toolbars: a light band, see-through like the dark one
        // The light style's colour replacements for the dark sheet:
        // stop:0 rgba(25,25,25) -> 239, stop:1 rgba(53,53,53) -> 203,
        // other rgba(25,25,25) -> 180, rgba(53,53,53) -> 210, rgba(66,66,66) -> 220
        f["toolbarDarkEnd"] = QStringLiteral("rgba(239, 239, 239, 127)");
        f["toolbarLightEnd"] = QStringLiteral("rgba(203, 203, 203, 75)");
        f["barEdge"] = QStringLiteral("rgba(180, 180, 180, 75)");
        f["boxFill"] = QStringLiteral("rgba(220, 220, 220, 50%)");
        f["menuBarItemHoverFill"] = QStringLiteral(
            "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(106, 106, 106, 255), stop:1 rgba(106, 106, 106, 75))");
        f["menuBarItemEdge"] = QStringLiteral("rgba(106, 106, 106, 127)");
    }
    f["checkedText"] = QStringLiteral("#ffffff");
    // More values from the application's dark style sheet, with light
    // counterparts
    f["menuBorder"] = dark ? QStringLiteral("rgb(20, 20, 20)") : QStringLiteral("rgb(25, 25, 25)");     // palette(shadow)
    f["menuSelectedEdge"] = QStringLiteral("rgba(147, 191, 236, 127)");
    f["menuDisabledFill"] = QStringLiteral("rgba(35, 35, 35, 127)");      // the light style keeps it too
    f["pressedEdge"] = dark ? QStringLiteral("rgba(25, 25, 25, 127)") : QStringLiteral("rgba(180, 180, 180, 127)");
    f["softLine"] = dark ? QStringLiteral("rgba(25, 25, 25, 50%)") : QStringLiteral("rgba(180, 180, 180, 50%)");   // tab bar / pane edge
    f["tabSelectedEdge"] = dark ? QStringLiteral("rgba(66, 66, 66, 75%)") : QStringLiteral("rgba(220, 220, 220, 75%)");
    f["disabledSelection"] = QStringLiteral("rgb(80, 80, 80)");      // palette Disabled Highlight
    // Disabled push buttons. Dark: a little lighter than the window
    // (rgb(53, 53, 53)), darker than an enabled button. Light: the window's
    // white, already the lightest.
    f["buttonDisabledFill"] = dark ? QStringLiteral("rgb(61, 61, 61)") : QStringLiteral("rgb(255, 255, 255)");
    f["sliderHandleBorder"] = dark ? QStringLiteral("rgba(25, 25, 25, 255)") : QStringLiteral("rgba(180, 180, 180, 255)");
    // Toolbar buttons and legend entries (the application's foo.qss): a
    // grey gradient with a groove border; light: the light style's values
    f["barButtonFill"] = dark
        ? QStringLiteral("qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(75, 75, 75, 255), stop:1 rgba(63, 63, 63, 255))")
        : QStringLiteral("qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(230, 230, 230, 255), stop:1 rgba(195, 195, 195, 255))");
    f["barButtonBorder"] = dark ? QStringLiteral("rgba(38, 38, 38, 255)") : QStringLiteral("rgba(182, 182, 182, 255)");
    // Group boxes: border, and the line under the title tab (as the
    // application's style sheet)
    f["groupBorder"] = dark ? QStringLiteral("rgba(25, 25, 25, 127)") : QStringLiteral("rgba(180, 180, 180, 127)");
    f["groupTitleLine"] = QStringLiteral("rgb(127, 127, 127)");
    // The toolbar gradients, one per side, from the two ends above: the
    // darker end is towards the window's edge
    const QString dark0 = f["toolbarDarkEnd"], light0 = f["toolbarLightEnd"];
    auto band = [&](const char *coords) {
        return QStringLiteral("qlineargradient(%1, stop:0 %2, stop:1 %3)")
            .arg(QLatin1String(coords), dark0, light0);
    };
    f["toolbarTopFill"] = band("x1:0, y1:0, x2:0, y2:1");
    f["toolbarBottomFill"] = band("x1:0, y1:1, x2:0, y2:0");
    f["toolbarLeftFill"] = band("x1:0, y1:0, x2:1, y2:0");
    f["toolbarRightFill"] = band("x1:1, y1:0, x2:0, y2:0");
    // Toolbar buttons and legend entries under the mouse: the band, as the
    // application's style sheets have it
    f["barButtonHoverFill"] = f["toolbarBottomFill"];      // both themes, as the application's sheets
    if (!dark)
        f["tabFill"] = f["toolbarBottomFill"];             // light: the light band
    return f;
}

// --- Images for the style sheet --------------------------------------------------
// Check boxes, radio buttons and the arrows of combo and spin boxes are drawn
// here, in the theme's colours, and saved as PNG files that the style sheet
// refers to. Each is saved at 1x and 2x ("@2x"), so it stays sharp on
// high-DPI screens. Drawing them avoids image files in the repository and a
// dependency on Qt's SVG module.

using Painter = std::function<void(QPainter &)>;

void saveImage(const QString &path, QSize size, const Painter &paint)
{
    for (int scale : { 1, 2 }) {
        QImage image(size * scale, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter p(&image);
        p.setRenderHint(QPainter::Antialiasing);
        p.scale(scale, scale);
        paint(p);
        p.end();
        QString file = path;
        if (scale == 2)
            file.insert(file.lastIndexOf(QLatin1Char('.')), QStringLiteral("@2x"));
        image.save(file);
    }
}

// A 10 x 10 chevron, pointing down or up
Painter chevron(const QColor &color, bool up)
{
    return [=](QPainter &p) {
        QPen pen(color, 1.4);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        p.setPen(pen);
        const double y1 = up ? 6.5 : 3.5, y2 = up ? 3.5 : 6.5;
        const QPointF points[] = { { 2.0, y1 }, { 5.0, y2 }, { 8.0, y1 } };
        p.drawPolyline(points, 3);
    };
}

// 12 x 12 dock widget title bar buttons: a cross (close), or two
// overlapping squares (float / dock again), like the window's title bar
Painter dockClose(const QColor &color)
{
    return [=](QPainter &p) {
        QPen pen(color, 1.5);
        pen.setCapStyle(Qt::RoundCap);
        p.setPen(pen);
        p.drawLine(QPointF(2, 2), QPointF(10, 10));
        p.drawLine(QPointF(10, 2), QPointF(2, 10));
    };
}

Painter dockFloat(const QColor &color)
{
    return [=](QPainter &p) {
        p.setPen(QPen(color, 1.3));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(4.5, 1.5, 6, 6), 1, 1);           // back
        p.drawRoundedRect(QRectF(1.5, 4.5, 6, 6), 1, 1);           // front
    };
}

// A chevron pointing right (closed tree branch), 10 x 10
Painter chevronRight(const QColor &color)
{
    return [=](QPainter &p) {
        QPen pen(color, 1.4);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        p.setPen(pen);
        const QPointF points[] = { { 3.5, 2.0 }, { 6.5, 5.0 }, { 3.5, 8.0 } };
        p.drawPolyline(points, 3);
    };
}

// A double chevron, 12 x 12: ">>" (pointing right) or pointing down - the
// toolbar extension button
Painter doubleChevron(const QColor &color, bool down)
{
    return [=](QPainter &p) {
        QPen pen(color, 1.4);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        p.setPen(pen);
        for (double offset : { 2.0, 6.0 }) {
            if (down) {
                const QPointF points[] = { { 2.5, offset + 0.5 }, { 6.0, offset + 3.5 }, { 9.5, offset + 0.5 } };
                p.drawPolyline(points, 3);
            } else {
                const QPointF points[] = { { offset + 0.5, 2.5 }, { offset + 3.5, 6.0 }, { offset + 0.5, 9.5 } };
                p.drawPolyline(points, 3);
            }
        }
    };
}

// Tree branch lines, 20 x 20 (stretched to the row by border-image): a line
// down the middle; "more": plus a line to the item; "end": the top half
// down to the item, then to it
enum class Branch { Line, More, End };
Painter branchLines(const QColor &color, Branch kind)
{
    return [=](QPainter &p) {
        p.setRenderHint(QPainter::Antialiasing, false);
        p.setPen(QPen(color, 1));
        p.drawLine(QPointF(9, 0), QPointF(9, kind == Branch::End ? 10 : 20));
        if (kind != Branch::Line)
            p.drawLine(QPointF(9, 10), QPointF(19, 10));
    };
}

// Splitter grip: three dots in a row (horizontal) or a column, 2 px each
Painter gripDots(const QColor &color, bool vertical)
{
    return [=](QPainter &p) {
        p.setPen(Qt::NoPen);
        p.setBrush(color);
        for (int i = 0; i < 3; ++i) {
            const qreal along = 2 + i * 5;          // 2, 7, 12 in a 14 px image
            p.drawEllipse(vertical ? QPointF(1.5, along) : QPointF(along, 1.5), 1.1, 1.1);
        }
    };
}

// A 16 x 16 check box: an empty rounded square, or filled with a check mark
// or a dash (indeterminate)
enum class Mark { None, Check, Dash };
Painter checkBox(const QColor &border, const QColor &fill, Mark mark, const QColor &markColor)
{
    return [=](QPainter &p) {
        const QRectF box(1.5, 1.5, 13, 13);
        p.setPen(mark == Mark::None ? QPen(border, 1.0) : Qt::NoPen);
        p.setBrush(fill);
        p.drawRoundedRect(mark == Mark::None ? box : box.adjusted(-0.5, -0.5, 0.5, 0.5), 3.5, 3.5);
        QPen pen(markColor, 1.8);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        p.setPen(pen);
        if (mark == Mark::Check) {
            const QPointF points[] = { { 4.6, 8.3 }, { 7.0, 10.6 }, { 11.6, 5.6 } };
            p.drawPolyline(points, 3);
        } else if (mark == Mark::Dash) {
            p.drawLine(QPointF(5.0, 8.0), QPointF(11.0, 8.0));
        }
    };
}

// A 16 x 16 radio button: an empty circle, or filled with a dot
Painter radioButton(const QColor &border, const QColor &fill, bool checked, const QColor &dot)
{
    return [=](QPainter &p) {
        p.setPen(checked ? Qt::NoPen : QPen(border, 1.0));
        p.setBrush(fill);
        p.drawEllipse(checked ? QRectF(1, 1, 14, 14) : QRectF(1.5, 1.5, 13, 13));
        if (checked) {
            p.setPen(Qt::NoPen);
            p.setBrush(dot);
            p.drawEllipse(QRectF(5, 5, 6, 6));
        }
    };
}

} // namespace

ModernStyle::~ModernStyle() = default;

ModernStyle::ModernStyle(Theme theme, QStyle *baseStyle)
    : QProxyStyle(baseStyle ? baseStyle : QStyleFactory::create(QStringLiteral("Fusion")))
    , m_theme(theme == Theme::Invalid ? Theme::System : theme)
    , m_accent()          // none: the palette's Highlight
    , m_systemPalette(QGuiApplication::palette())
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    // Follow the operating system when it switches between light and dark
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this] {
        if (m_theme == Theme::System)
            apply();
    });
#endif
}

bool ModernStyle::isDark() const
{
    switch (m_theme) {
    case Theme::Light:
        return false;
    case Theme::Dark:
        return true;
    case Theme::System:
    case Theme::Invalid:
        break;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    const Qt::ColorScheme scheme = QGuiApplication::styleHints()->colorScheme();
    if (scheme != Qt::ColorScheme::Unknown)
        return scheme == Qt::ColorScheme::Dark;
#endif
    // Older Qt, or the system doesn't say: judge by its window colour
    return m_systemPalette.color(QPalette::Window).lightness() < 128;
}

void ModernStyle::setTheme(Theme theme)
{
    if (theme == Theme::Invalid)
        return;
    m_theme = theme;
    apply();
}

QStringList ModernStyle::getThemeNames() const
{
    // From the enum itself (Q_ENUM), so a theme added later shows up here too
    const QMetaEnum themes = QMetaEnum::fromType<Theme>();
    QStringList names;
    for (int i = 0; i < themes.keyCount(); ++i) {
        if (Theme(themes.value(i)) != Theme::Invalid)
            names.append(QString::fromLatin1(themes.key(i)));
    }
    return names;
}

ModernStyle::Theme ModernStyle::themeFromName(const QString &name)
{
    const QMetaEnum themes = QMetaEnum::fromType<Theme>();
    for (int i = 0; i < themes.keyCount(); ++i) {
        if (name.compare(QLatin1String(themes.key(i)), Qt::CaseInsensitive) == 0)
            return Theme(themes.value(i));
    }
    return Theme::Invalid;
}

QString ModernStyle::themeName(Theme theme)
{
    if (theme == Theme::Invalid)
        return {};
    return QString::fromLatin1(QMetaEnum::fromType<Theme>().valueToKey(int(theme)));
}

int ModernStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    if (metric == PM_ToolBarExtensionExtent)
        return 29;
    return QProxyStyle::pixelMetric(metric, option, widget);
}

void ModernStyle::setDisabledButtonCursor(bool on)
{
    if (on == disabledButtonCursor())
        return;
    if (on) {
        m_disabledCursorFilter = new DisabledButtonCursor(this);
    } else {
        delete m_disabledCursorFilter;
        m_disabledCursorFilter = nullptr;
    }
}

int ModernStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget,
                           QStyleHintReturn *returnData) const
{
    if (hint == SH_ComboBox_AllowWheelScrolling) {
        // The combo box's own "wheelScrolling" property, else comboWheelScrolling().
        // Off: QComboBox ignores the wheel, so it goes on to the panel or page
        // under it, which scrolls.
        const QVariant own = widget ? widget->property("wheelScrolling") : QVariant();
        const bool allowed = own.isValid() ? own.toBool() : m_comboWheelScrolling;
        if (!allowed)
            return 0;
    }
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

QIcon ModernStyle::standardIcon(StandardPixmap icon, const QStyleOption *option, const QWidget *widget) const
{
    // The toolbar's ">>" button, drawn in the current theme's text colour
    // (QToolBar asks again when the theme changes)
    if (icon == SP_ToolBarHorizontalExtensionButton || icon == SP_ToolBarVerticalExtensionButton) {
        const QString file = writeImages()
            + (icon == SP_ToolBarHorizontalExtensionButton ? QStringLiteral("toolbar-extension-h.png")
                                                           : QStringLiteral("toolbar-extension-v.png"));
        return QIcon(file);           // the @2x file next to it is used on high-DPI screens
    }
    return QProxyStyle::standardIcon(icon, option, widget);
}

void ModernStyle::setObjectFontPointSize(const QString &objectName, qreal pointSize)
{
    if (pointSize > 0)
        m_objectFontSizes.insert(objectName, pointSize);
    else
        m_objectFontSizes.remove(objectName);
    // Widgets that exist already; new ones get it in polish()
    for (QWidget *widget : QApplication::allWidgets()) {
        if (widget->objectName() == objectName)
            applyFontSize(widget);
    }
}

// The widget's own "fontPointSize" property wins over a size for its object
// name. A widget that had a size by name and no longer has one keeps it
// (fonts don't remember where they came from); set another size instead.
void ModernStyle::applyFontSize(QWidget *widget) const
{
    bool ok = false;
    qreal size = widget->property("fontPointSize").toReal(&ok);
    if (!ok || size <= 0)
        size = m_objectFontSizes.value(widget->objectName(), 0);
    if (size <= 0)
        return;
    QFont font = widget->font();
    if (qFuzzyCompare(font.pointSizeF(), size))
        return;                       // already (polish() runs again on theme changes)
    font.setPointSizeF(size);
    widget->setFont(font);
}

void ModernStyle::setExtraStyleSheet(const QString &rules)
{
    m_extraSheet = rules;
    apply();
}

void ModernStyle::setAccentColor(const QColor &color)
{
    m_accent = color;
    apply();
}

ModernStyle::Colors ModernStyle::colors() const
{
    return themeColors(isDark(), m_accent);
}

QPalette ModernStyle::standardPalette() const
{
    // The palette exactly as given (themePalette()); a custom accent replaces
    // Highlight and Link
    QPalette p = themePalette(isDark());
    const Colors c = colors();
    if (m_accent.isValid()) {
        p.setColor(QPalette::Highlight, c["accent"]);
        p.setColor(QPalette::Link, c["accent"]);
    }
    // Roles the given palettes leave out
    p.setColor(QPalette::PlaceholderText, c["textDisabled"]);
    p.setColor(QPalette::LinkVisited, p.color(QPalette::Link).darker(120));
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    p.setColor(QPalette::Accent, p.color(QPalette::Highlight));
#endif
    return p;
}

void ModernStyle::polish(QPalette &palette)
{
    palette = standardPalette();
}

void ModernStyle::polish(QApplication *app)
{
    QProxyStyle::polish(app);
    // QApplication::setStyle() lands here: put the palette and the style
    // sheet onto the application
    if (app)
        apply();
}

namespace {

// The tab bars QMainWindow makes for tabbed dock widgets: a QTabBar whose
// parent is the main window itself
bool isDockTabBar(const QWidget *widget)
{
    return qobject_cast<const QTabBar *>(widget) && qobject_cast<QMainWindow *>(widget->parentWidget());
}

// Put each dock widget's icon on its tab. QMainWindow stores the dock
// widget's address as the tab's data; it is only compared with the main
// window's dock widgets, never used as a pointer on its own.
void syncDockTabIcons(QTabBar *tabs)
{
    auto *mainWindow = qobject_cast<QMainWindow *>(tabs->parentWidget());
    if (!mainWindow)
        return;
    const QList<QDockWidget *> docks = mainWindow->findChildren<QDockWidget *>();
    for (int i = 0; i < tabs->count(); ++i) {
        const quintptr id = tabs->tabData(i).value<quintptr>();
        for (QDockWidget *dock : docks) {
            if (reinterpret_cast<quintptr>(dock) != id)
                continue;
            const QIcon icon = dock->testAttribute(Qt::WA_SetWindowIcon) ? dock->windowIcon() : QIcon();
            if (tabs->tabIcon(i).cacheKey() != icon.cacheKey())
                tabs->setTabIcon(i, icon);
            break;
        }
    }
}

// Again after the current event: QMainWindow rebuilds its tab bars when dock
// widgets are tabbed, moved or floated, and icons may change at any time
void syncDockTabIconsLater(QTabBar *tabs)
{
    if (tabs->property("_modernIconSyncPending").toBool())
        return;
    tabs->setProperty("_modernIconSyncPending", true);
    QTimer::singleShot(0, tabs, [tabs = QPointer<QTabBar>(tabs)] {
        if (!tabs)
            return;
        tabs->setProperty("_modernIconSyncPending", false);
        syncDockTabIcons(tabs);
    });
}

} // namespace

void ModernStyle::polish(QWidget *widget)
{
    QProxyStyle::polish(widget);

    // The application replaced ModernStyle's style sheet (qApp->setStyleSheet()):
    // its rules are gone, only the palette is left. Say so once. QApplication
    // keeps a shared copy of the sheet, so a different one is different data;
    // each new sheet is looked at once (appending to ours is fine).
    const QString appSheet = qApp->styleSheet();
    if (!m_warnedSheetReplaced && !m_applying && !m_appliedSheet.isEmpty()
        && appSheet.constData() != m_appliedSheet.constData()
        && appSheet.constData() != m_checkedSheet.constData()) {
        m_checkedSheet = appSheet;
        if (!appSheet.startsWith(m_appliedSheet)) {
            m_warnedSheetReplaced = true;
            qWarning("ModernStyle: the application's style sheet was replaced with "
                     "qApp->setStyleSheet(), which removes ModernStyle's own rules. Set style "
                     "sheets on your widgets instead, or append to "
                     "ModernStyle::currentStyleSheet().");
        }
    }
    // Menus: transparent outside their rounded border, so the corners are
    // really round. Must be set before the menu's window exists; polish()
    // runs before a widget is first shown.
    if (qobject_cast<QMenu *>(widget) && !widget->testAttribute(Qt::WA_WState_Created))
        widget->setAttribute(Qt::WA_TranslucentBackground);

    // The toolbar's ">>" button sets its icon once; give it the current
    // theme's (polish() runs again after every theme change)
    if (widget->objectName() == QLatin1String("qt_toolbar_ext_button")) {
        if (auto *button = qobject_cast<QToolButton *>(widget)) {
            auto *toolBar = qobject_cast<QToolBar *>(button->parentWidget());
            const bool vertical = toolBar && toolBar->orientation() == Qt::Vertical;
            button->setIcon(standardIcon(vertical ? SP_ToolBarVerticalExtensionButton
                                                  : SP_ToolBarHorizontalExtensionButton));
        }
    }

    // Font size from the "fontPointSize" property or the object name
    applyFontSize(widget);

    // Combo boxes: watch their drop-down list, to keep it within the
    // maximum height (eventFilter())
    if (auto *combo = qobject_cast<QComboBox *>(widget))
        combo->view()->window()->installEventFilter(this);

    // Dock widgets: the title bar with icon (DockTitleBar), and watch for
    // icon changes, which their tabs must show
    if (auto *dock = qobject_cast<QDockWidget *>(widget)) {
        if (m_dockTitleBars)
            DockTitleBar::install(dock);
        dock->installEventFilter(this);
    }
    // Tabbed dock widgets: icons on the tabs
    if (isDockTabBar(widget)) {
        widget->installEventFilter(this);
        syncDockTabIconsLater(static_cast<QTabBar *>(widget));
    }
}

void ModernStyle::unpolish(QWidget *widget)
{
    if (auto *combo = qobject_cast<QComboBox *>(widget))
        combo->view()->window()->removeEventFilter(this);
    if (qobject_cast<QDockWidget *>(widget) || isDockTabBar(widget))
        widget->removeEventFilter(this);
    QProxyStyle::unpolish(widget);
}

bool ModernStyle::eventFilter(QObject *watched, QEvent *event)
{
    // Dock widget tabs: QMainWindow refills them (titles) without icons; put
    // the icons back before they're painted. Cheap when nothing changed.
    if (event->type() == QEvent::Paint || event->type() == QEvent::Show) {
        auto *tabs = qobject_cast<QTabBar *>(watched);
        if (tabs && isDockTabBar(tabs))
            syncDockTabIconsLater(tabs);
    }
    // A dock widget's icon changed: its tab must show the new one
    if (event->type() == QEvent::WindowIconChange) {
        if (auto *dock = qobject_cast<QDockWidget *>(watched)) {
            if (auto *mainWindow = qobject_cast<QMainWindow *>(dock->parentWidget())) {
                for (QTabBar *tabs : mainWindow->findChildren<QTabBar *>(QString(), Qt::FindDirectChildrenOnly))
                    syncDockTabIconsLater(tabs);
            }
        }
    }

    // A combo box's drop-down list is being shown: QComboBox has sized it for
    // its items; make it no taller than the maximum height (the combo box's
    // own "popupMaxHeight" property, else comboPopupMaxHeight()). The list
    // inside then scrolls.
    if (event->type() == QEvent::Show) {
        auto *popup = qobject_cast<QWidget *>(watched);
        auto *combo = popup ? qobject_cast<QComboBox *>(popup->parentWidget()) : nullptr;
        if (combo) {
            bool ok = false;
            int maxHeight = combo->property("popupMaxHeight").toInt(&ok);
            if (!ok)
                maxHeight = m_comboPopupMaxHeight;
            if (maxHeight > 0 && popup->height() > maxHeight) {
                QRect geometry = popup->geometry();
                // Opened above the combo box (no room below): keep its bottom edge
                const bool above = geometry.bottom() < combo->mapToGlobal(QPoint(0, 0)).y();
                if (above)
                    geometry.setTop(geometry.bottom() - maxHeight + 1);
                else
                    geometry.setHeight(maxHeight);
                popup->setGeometry(geometry);
            }
        }
    }
    return QProxyStyle::eventFilter(watched, event);
}

QString ModernStyle::styleSheet() const
{
    QFile file(QStringLiteral(":/modernstyle/modernstyle.qss"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Without the sheet only the palette applies: plain Fusion widgets
        qWarning("ModernStyle: the style sheet resource :/modernstyle/modernstyle.qss is missing, "
                 "so only the palette is applied. Link the FramelessWindow::FramelessWindow CMake "
                 "target (it brings the resource), don't compile the library's .cpp files into "
                 "your own target.");
        return {};
    }
    QString qss = QString::fromUtf8(file.readAll());
    // The application's own rules last, so they win over the ones above
    if (!m_extraSheet.isEmpty())
        qss += QStringLiteral("\n/* --- setExtraStyleSheet() --- */\n") + m_extraSheet;

    // @icons/<name>.png: the images drawn for the current colours
    qss.replace(QStringLiteral("@icons/"), writeImages());

    // Every @name and its value: the colours, then the button and tab fills
    const Colors c = colors();
    QHash<QString, QString> values = themeFills(isDark(), c);
    for (auto it = c.cbegin(); it != c.cend(); ++it) {
        const QColor color = it.value();
        values[it.key()] = color.alpha() == 255
            ? color.name()
            : QStringLiteral("rgba(%1, %2, %3, %4)")
                  .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
    }

    // Replace them, longest names first, so @accentHover isn't taken for
    // @accent followed by "Hover"
    QStringList names = values.keys();
    std::sort(names.begin(), names.end(),
              [](const QString &a, const QString &b) { return a.size() > b.size(); });
    for (const QString &name : names)
        qss.replace(QLatin1Char('@') + name, values[name]);
    return qss;
}

QString ModernStyle::writeImages() const
{
    if (!m_imageDir) {
        m_imageDir = std::make_unique<QTemporaryDir>();
        m_imageDir->setAutoRemove(true);
    }
    const Colors c = colors();

    // File names start with a key made from the colours: Qt caches images by
    // file name, so new colours need new names
    const QString key = QStringLiteral("%1-%2").arg(isDark() ? "dark" : "light", c["accent"].name().mid(1));
    const QString prefix = QDir::fromNativeSeparators(m_imageDir->path()) + QLatin1Char('/') + key + QLatin1Char('-');
    if (QFile::exists(prefix + QStringLiteral("chevron-down.png")))
        return prefix;    // already drawn

    const QSize arrow(10, 10), indicator(16, 16), dockButton(12, 12);
    auto save = [&](const char *name, QSize size, const Painter &paint) {
        saveImage(prefix + QLatin1String(name) + QStringLiteral(".png"), size, paint);
    };
    save("chevron-down", arrow, chevron(c["text"], false));
    save("chevron-up", arrow, chevron(c["text"], true));
    save("chevron-down-disabled", arrow, chevron(c["textDisabled"], false));
    save("chevron-up-disabled", arrow, chevron(c["textDisabled"], true));

    save("checkbox", indicator, checkBox(c["borderStrong"], c["surface"], Mark::None, {}));
    save("checkbox-hover", indicator, checkBox(c["textSecondary"], c["surface"], Mark::None, {}));
    save("checkbox-checked", indicator, checkBox({}, c["accent"], Mark::Check, c["accentText"]));
    save("checkbox-checked-hover", indicator, checkBox({}, c["accentHover"], Mark::Check, c["accentText"]));
    save("checkbox-indeterminate", indicator, checkBox({}, c["accent"], Mark::Dash, c["accentText"]));
    save("checkbox-disabled", indicator, checkBox(c["border"], c["window"], Mark::None, {}));
    save("checkbox-checked-disabled", indicator, checkBox({}, c["border"], Mark::Check, c["textDisabled"]));
    save("checkbox-indeterminate-disabled", indicator, checkBox({}, c["border"], Mark::Dash, c["textDisabled"]));

    save("radio", indicator, radioButton(c["borderStrong"], c["surface"], false, {}));
    save("radio-hover", indicator, radioButton(c["textSecondary"], c["surface"], false, {}));
    save("radio-checked", indicator, radioButton({}, c["accent"], true, c["accentText"]));
    save("radio-checked-hover", indicator, radioButton({}, c["accentHover"], true, c["accentText"]));
    save("radio-disabled", indicator, radioButton(c["border"], c["window"], false, {}));
    save("radio-checked-disabled", indicator, radioButton({}, c["border"], true, c["textDisabled"]));
    // Pressed (mouse held down): accent edge, or the darker accent when checked
    save("checkbox-pressed", indicator, checkBox(c["accent"], c["surface"], Mark::None, {}));
    save("checkbox-checked-pressed", indicator, checkBox({}, c["accentPressed"], Mark::Check, c["accentText"]));
    save("checkbox-indeterminate-pressed", indicator, checkBox({}, c["accentPressed"], Mark::Dash, c["accentText"]));
    save("radio-pressed", indicator, radioButton(c["accent"], c["surface"], false, {}));
    save("radio-checked-pressed", indicator, radioButton({}, c["accentPressed"], true, c["accentText"]));

    // Tree views: branch lines and open/closed arrows
    const QSize branch(20, 20);
    save("branch-line", branch, branchLines(c["borderStrong"], Branch::Line));
    save("branch-more", branch, branchLines(c["borderStrong"], Branch::More));
    save("branch-end", branch, branchLines(c["borderStrong"], Branch::End));
    save("branch-closed", arrow, chevronRight(c["textSecondary"]));
    save("branch-open", arrow, chevron(c["textSecondary"], false));

    save("toolbar-extension-h", QSize(12, 12), doubleChevron(c["text"], false));   // standardIcon()
    save("toolbar-extension-v", QSize(12, 12), doubleChevron(c["text"], true));
    save("grip-vertical", QSize(3, 14), gripDots(c["grip"], true));     // QSplitter::handle:horizontal
    save("grip-horizontal", QSize(14, 3), gripDots(c["grip"], false));  // QSplitter::handle:vertical
    save("dock-close", dockButton, dockClose(c["text"]));
    save("dock-float", dockButton, dockFloat(c["text"]));
    return prefix;
}

void ModernStyle::apply()
{
    // setStyleSheet() re-polishes the application, which calls polish(app)
    // again: don't loop
    if (m_applying || !qApp)
        return;
    m_applying = true;
    QApplication::setPalette(standardPalette());
    m_appliedSheet = styleSheet();
    qApp->setStyleSheet(m_appliedSheet);
    m_warnedSheetReplaced = false;
    m_applying = false;
    emit themeChanged(isDark());
}
