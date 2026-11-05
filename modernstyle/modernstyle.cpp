// MIT License, see LICENSE. Based on Qt-Frameless-Window-DarkStyle by Juergen
// Skrotzky (https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle).

#include "modernstyle.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QMenu>
#include <QStyleFactory>
#include <QStyleHints>
#include <QTemporaryDir>

#include <algorithm>
#include <functional>

namespace {

// Light and dark colours. The style sheet refers to them as @name
// (e.g. @border); the palette is built from them too.
QHash<QString, QColor> themeColors(bool dark, const QColor &accent)
{
    QHash<QString, QColor> c;
    if (dark) {
        c["window"] = QColor("#202020");          // window background
        c["surface"] = QColor("#2b2b2b");         // text fields, lists, tables
        c["surfaceAlt"] = QColor("#323232");      // alternating rows, headers
        c["control"] = QColor("#2f2f2f");         // buttons
        c["controlHover"] = QColor("#3a3a3a");
        c["controlPressed"] = QColor("#272727");
        c["border"] = QColor("#3d3d3d");
        c["borderStrong"] = QColor("#525252");
        c["text"] = QColor("#f2f2f2");
        c["textSecondary"] = QColor("#a6a6a6");
        c["textDisabled"] = QColor("#6e6e6e");
        c["menu"] = QColor("#2c2c2c");
        c["tooltip"] = QColor("#2c2c2c");
        c["tab"] = QColor("#2b2b2b");             // unselected tabs, scroll bar tracks
        c["tabHover"] = QColor("#363636");
    } else {
        c["window"] = QColor("#f3f3f3");
        c["surface"] = QColor("#ffffff");
        c["surfaceAlt"] = QColor("#f7f7f7");
        c["control"] = QColor("#fbfbfb");
        c["controlHover"] = QColor("#f0f0f0");
        c["controlPressed"] = QColor("#e6e6e6");
        c["border"] = QColor("#d6d6d6");
        c["borderStrong"] = QColor("#b3b3b3");
        c["text"] = QColor("#1c1c1c");
        c["textSecondary"] = QColor("#5c5c5c");
        c["textDisabled"] = QColor("#a3a3a3");
        c["menu"] = QColor("#ffffff");
        c["tooltip"] = QColor("#ffffff");
        c["tab"] = QColor("#e7e7e7");
        c["tabHover"] = QColor("#dedede");
    }
    // The accent, a little lighter on dark backgrounds so it stands out
    c["accent"] = dark ? accent.lighter(115) : accent;
    c["accentHover"] = dark ? accent.lighter(130) : accent.darker(110);
    c["accentPressed"] = dark ? accent : accent.darker(125);
    c["accentText"] = QColor("#ffffff");      // text on the accent colour
    QColor selection = c["accent"];
    selection.setAlpha(dark ? 90 : 60);
    c["selection"] = selection;               // e.g. hovered menu items
    return c;
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
    , m_theme(theme)
    , m_accent(QColor("#2f6fed"))
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
    m_theme = theme;
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
    const Colors c = colors();
    QPalette p;
    p.setColor(QPalette::Window, c["window"]);
    p.setColor(QPalette::WindowText, c["text"]);
    p.setColor(QPalette::Base, c["surface"]);
    p.setColor(QPalette::AlternateBase, c["surfaceAlt"]);
    p.setColor(QPalette::ToolTipBase, c["tooltip"]);
    p.setColor(QPalette::ToolTipText, c["text"]);
    p.setColor(QPalette::PlaceholderText, c["textSecondary"]);
    p.setColor(QPalette::Text, c["text"]);
    p.setColor(QPalette::Button, c["control"]);
    p.setColor(QPalette::ButtonText, c["text"]);
    p.setColor(QPalette::BrightText, QColor("#ff5c5c"));
    // Fusion draws bevels and frames with these
    p.setColor(QPalette::Light, c["controlHover"]);
    p.setColor(QPalette::Midlight, c["border"]);
    p.setColor(QPalette::Mid, c["borderStrong"]);
    p.setColor(QPalette::Dark, c["borderStrong"]);
    p.setColor(QPalette::Shadow, QColor(0, 0, 0, isDark() ? 160 : 60));
    p.setColor(QPalette::Highlight, c["accent"]);
    p.setColor(QPalette::HighlightedText, c["accentText"]);
    p.setColor(QPalette::Link, c["accent"]);
    p.setColor(QPalette::LinkVisited, c["accentPressed"]);
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    p.setColor(QPalette::Accent, c["accent"]);
#endif

    // Disabled widgets
    for (QPalette::ColorRole role : { QPalette::WindowText, QPalette::Text, QPalette::ButtonText })
        p.setColor(QPalette::Disabled, role, c["textDisabled"]);
    p.setColor(QPalette::Disabled, QPalette::Highlight, c["borderStrong"]);
    p.setColor(QPalette::Disabled, QPalette::HighlightedText, c["textDisabled"]);
    p.setColor(QPalette::Disabled, QPalette::Button, c["window"]);
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

void ModernStyle::polish(QWidget *widget)
{
    QProxyStyle::polish(widget);
    // Menus: transparent outside their rounded border, so the corners are
    // really round. Must be set before the menu's window exists; polish()
    // runs before a widget is first shown.
    if (qobject_cast<QMenu *>(widget) && !widget->testAttribute(Qt::WA_WState_Created))
        widget->setAttribute(Qt::WA_TranslucentBackground);
}

QString ModernStyle::styleSheet() const
{
    QFile file(QStringLiteral(":/modernstyle/modernstyle.qss"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    QString qss = QString::fromUtf8(file.readAll());

    // @icons/<name>.png: the images drawn for the current colours
    qss.replace(QStringLiteral("@icons/"), writeImages());

    // Replace each @name with its colour. Longest names first, so @accentHover
    // isn't taken for @accent followed by "Hover".
    const Colors c = colors();
    QStringList names = c.keys();
    std::sort(names.begin(), names.end(),
              [](const QString &a, const QString &b) { return a.size() > b.size(); });
    for (const QString &name : names) {
        const QColor color = c[name];
        const QString value = color.alpha() == 255
            ? color.name()
            : QStringLiteral("rgba(%1, %2, %3, %4)")
                  .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
        qss.replace(QLatin1Char('@') + name, value);
    }
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

    const QSize arrow(10, 10), indicator(16, 16);
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
    qApp->setStyleSheet(styleSheet());
    m_applying = false;
    emit themeChanged(isDark());
}
