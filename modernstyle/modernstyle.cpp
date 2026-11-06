// MIT License, see LICENSE. Based on Qt-Frameless-Window-DarkStyle by Juergen
// Skrotzky (https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle).

#include "modernstyle.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QAbstractItemView>
#include <QComboBox>
#include <QMenu>
#include <QMetaEnum>
#include <QStyleFactory>
#include <QStyleHints>
#include <QTemporaryDir>

#include <algorithm>
#include <functional>

namespace {

// Light and dark colours. The style sheet refers to them as @name
// (e.g. @border); the palette is built from them too.
//
// The dark theme follows Bootswatch's "Slate" theme for Bootstrap
// (https://bootswatch.com/slate/, dist/slate/_variables.scss), so a Qt app
// and a web front end using Slate look alike. Slate's names and Sass
// expressions are noted next to each colour.
QHash<QString, QColor> themeColors(bool dark, const QColor &accent)
{
    QHash<QString, QColor> c;
    if (dark) {
        c["window"] = QColor("#272b30");          // $body-bg ($gray-900)
        c["surface"] = QColor("#32383e");         // $list-group-bg, $card-bg: lighten($body-bg, 5%)
        c["surfaceAlt"] = QColor("#3c4248");      // table stripes: rgba($white, .05) over that
        c["control"] = QColor("#3a3f44");         // $gray-800 (Slate's buttons, dropdowns)
        c["controlHover"] = QColor("#3e444c");    // $list-group-hover-bg: lighten($body-bg, 10%)
        c["controlPressed"] = QColor("#2b2f32");  // shade-color($gray-800, 26%)
        c["border"] = QColor("#101113");          // rgba($black, .6) over $body-bg: all of Slate's borders
        c["borderStrong"] = QColor("#52575c");    // $gray-700
        c["groove"] = QColor("#7a8288");          // slider track, scroll bar handle: $gray-600
        c["grooveHover"] = QColor("#aaaaaa");     // scroll bar handle under the mouse: $gray-500
        c["grip"] = QColor("#aaaaaa");            // the dots on splitter handles
        c["text"] = QColor("#e9ecef");            // $gray-200 (headings, active items)
        c["textSecondary"] = QColor("#aaaaaa");   // $body-color
        c["textDisabled"] = QColor("#7a8288");    // $gray-600
        c["menu"] = QColor("#3a3f44");            // $dropdown-bg
        c["tooltip"] = QColor("#32383e");         // $popover-bg
        c["tab"] = QColor("#1c1e22");             // scroll bar tracks: $progress-bg, darken($gray-900, 5%)
        c["tabHover"] = QColor("#3e444c");
        c["panel"] = QColor("#32383e");           // dock widgets: $card-bg, lighten($body-bg, 5%)
        c["panelHeader"] = QColor("#515960");     // their title bar: $card-cap-bg, lighten($gray-800, 10%)
    } else {
        c["window"] = QColor("#f3f3f3");
        c["surface"] = QColor("#ffffff");
        c["surfaceAlt"] = QColor("#f7f7f7");
        c["control"] = QColor("#fbfbfb");
        c["controlHover"] = QColor("#f0f0f0");
        c["controlPressed"] = QColor("#e6e6e6");
        c["border"] = QColor("#d6d6d6");
        c["borderStrong"] = QColor("#b3b3b3");
        c["groove"] = QColor("#bdbdbd");
        c["grooveHover"] = QColor("#8f8f8f");
        c["grip"] = QColor("#8f8f8f");
        c["text"] = QColor("#1c1c1c");
        c["textSecondary"] = QColor("#5c5c5c");
        c["textDisabled"] = QColor("#a3a3a3");
        c["menu"] = QColor("#ffffff");
        c["tooltip"] = QColor("#ffffff");
        c["tab"] = QColor("#e7e7e7");
        c["tabHover"] = QColor("#dedede");
        c["panel"] = QColor("#ffffff");
        c["panelHeader"] = QColor("#e8e8e8");
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

// A vertical gradient as Bootstrap's gradient-y-three-colors() makes it:
// top colour, a middle colour reached at `position` (0..1), bottom colour
QString verticalGradient(const char *top, const char *middle, double position, const char *bottom)
{
    return QStringLiteral("qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %1, stop:%2 %3, stop:1 %4)")
        .arg(QLatin1String(top)).arg(position).arg(QLatin1String(middle)).arg(QLatin1String(bottom));
}

// Fills for buttons and tabs. The style sheet refers to them as @name like
// the colours, but they can be gradients. Dark (Slate): the gradients of
// Slate's .btn and .nav-tabs (_bootswatch.scss) - raised, darker when
// hovered and pressed, near-black border. Light: plain colours.
QHash<QString, QString> themeFills(bool dark, const QHash<QString, QColor> &c)
{
    QHash<QString, QString> f;
    if (dark) {
        // btn-shadow($gray-800): tint 5% -> $gray-800 at 20% -> shade 20%
        const QString raised = verticalGradient("#44494d", "#3a3f44", 0.2, "#2e3236");
        // btn-shadow-inverse($gray-800): shade 16% -> shade 10% at 40% -> shade 6%
        const QString sunken = verticalGradient("#313539", "#34393d", 0.4, "#373b40");
        f["buttonFill"] = raised;
        f["buttonHoverFill"] = sunken;
        // btn-shadow-inverse-dark($gray-800): shade 36% -> 30% at 40% -> 26%
        f["buttonPressedFill"] = verticalGradient("#25282c", "#292c30", 0.4, "#2b2f32");
        f["buttonBorder"] = QStringLiteral("rgba(0, 0, 0, 153)");      // rgba($black, .6)
        f["buttonHoverBorder"] = f["buttonBorder"];
        f["buttonText"] = QStringLiteral("#ffffff");
        // Slate's tabs: unselected sunken, hovered and selected raised
        f["tabFill"] = sunken;
        f["tabHoverFill"] = raised;
        f["tabSelectedFill"] = raised;
        // Tool buttons (toolbars) are flat until the mouse is over them; then
        // they light up: Slate's raised button, a step brighter (tint 12%
        // instead of 5% at the top). Checked: pressed in (btn-shadow-inverse-
        // dark), lighting up the same way when the mouse is over them.
        f["toolHoverFill"] = verticalGradient("#53575c", "#474c50", 0.2, "#3a3f44");
        f["toolCheckedFill"] = verticalGradient("#25282c", "#292c30", 0.4, "#2b2f32");
        f["toolCheckedHoverFill"] = f["toolHoverFill"];
    } else {
        f["buttonFill"] = c["control"].name();
        f["buttonHoverFill"] = c["controlHover"].name();
        f["buttonPressedFill"] = c["controlPressed"].name();
        f["buttonBorder"] = c["border"].name();
        f["buttonHoverBorder"] = c["borderStrong"].name();
        f["buttonText"] = c["text"].name();
        f["tabFill"] = c["tab"].name();
        f["tabHoverFill"] = c["tabHover"].name();
        f["tabSelectedFill"] = c["window"].name();
        f["toolHoverFill"] = c["controlHover"].name();
        QColor checked = c["accent"], checkedHover = c["accent"];
        checked.setAlpha(45);
        checkedHover.setAlpha(80);     // lights up: a stronger tint of the accent
        auto rgba = [](const QColor &color) {
            return QStringLiteral("rgba(%1, %2, %3, %4)")
                .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
        };
        f["toolCheckedFill"] = rgba(checked);
        f["toolCheckedHoverFill"] = rgba(checkedHover);
    }
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

    // Combo boxes: watch their drop-down list, to keep it within the
    // maximum height (eventFilter())
    if (auto *combo = qobject_cast<QComboBox *>(widget))
        combo->view()->window()->installEventFilter(this);
}

void ModernStyle::unpolish(QWidget *widget)
{
    if (auto *combo = qobject_cast<QComboBox *>(widget))
        combo->view()->window()->removeEventFilter(this);
    QProxyStyle::unpolish(widget);
}

bool ModernStyle::eventFilter(QObject *watched, QEvent *event)
{
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
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    QString qss = QString::fromUtf8(file.readAll());

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
    qApp->setStyleSheet(styleSheet());
    m_applying = false;
    emit themeChanged(isDark());
}
