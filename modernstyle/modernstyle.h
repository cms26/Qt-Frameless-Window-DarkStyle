// ModernStyle - a flat, rounded Qt Widgets style with a light and a dark
// theme. By default it follows the operating system's light/dark setting and
// switches when that changes (Qt 6.5 or newer; older Qt picks the theme from
// the system palette at start-up).
//
//   QApplication app(argc, argv);
//   auto *style = new ModernStyle;            // or ModernStyle(ModernStyle::Theme::Dark)
//   QApplication::setStyle(style);            // the application takes ownership
//   ...
//   style->setTheme(ModernStyle::Theme::Light);   // switch at any time
//
// It is Qt's Fusion style with a tuned palette plus a style sheet
// (modernstyle.qss) for rounded controls, hover states and the accent colour.
//
// MIT License, see LICENSE. Based on DarkStyle by Juergen Skrotzky
// (https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle).

#pragma once

#include <QColor>
#include <QHash>
#include <QPalette>
#include <QProxyStyle>
#include <QString>
#include <QStringList>

#include <memory>

class QTemporaryDir;

class ModernStyle : public QProxyStyle
{
    Q_OBJECT

public:
    enum class Theme {
        System, // follow the operating system's light/dark setting
        Light,
        Dark,
        Invalid, // not a theme: what themeFromName() returns for an unknown name
    };
    Q_ENUM(Theme)

    explicit ModernStyle(Theme theme = Theme::System, QStyle *baseStyle = nullptr);
    ~ModernStyle() override;

    // The chosen theme, and whether that currently means dark
    Theme theme() const { return m_theme; }
    bool isDark() const;

    // Switch the theme of the running application. Theme::Invalid is ignored.
    void setTheme(Theme theme);

    // The themes by name, for a settings dialog or a command line option:
    // "System", "Light", "Dark" (in the enum's order, without Invalid)
    QStringList getThemeNames() const;

    // Name <-> theme. themeFromName() ignores case and returns Theme::Invalid
    // for a name that isn't a theme; themeName(Theme::Invalid) is empty.
    static Theme themeFromName(const QString &name);
    static QString themeName(Theme theme);

    // The colour of selections, default buttons, focus frames, sliders and
    // progress bars. Default: the theme palette's Highlight (dark
    // rgb(42, 130, 218), light rgb(90, 154, 209)); setAccentColor(QColor())
    // goes back to that.
    QColor accentColor() const { return standardPalette().color(QPalette::Highlight); }
    void setAccentColor(const QColor &color);

    // The tallest a combo box's drop-down list may be, in pixels; longer
    // lists scroll. 0 (default): no pixel limit, only QComboBox::
    // maxVisibleItems() (10 unless changed). A combo box can have its own
    // limit: combo->setProperty("popupMaxHeight", 200).
    int comboPopupMaxHeight() const { return m_comboPopupMaxHeight; }
    void setComboPopupMaxHeight(int pixels) { m_comboPopupMaxHeight = qMax(0, pixels); }

    // Dock widgets: give each one ModernStyle's title bar (DockTitleBar), which
    // shows the dock's icon (QDockWidget::setWindowIcon()) docked and
    // floating. On by default; dock widgets with a title bar widget of their
    // own keep it. Set before the dock widgets are shown. Either way, tabbed
    // dock widgets show their icons in the tabs.
    bool dockTitleBars() const { return m_dockTitleBars; }
    void setDockTitleBars(bool on) { m_dockTitleBars = on; }

    // Your own application-wide style sheet rules, added after ModernStyle's
    // so they win where both style the same thing. They may use the same
    // @names as modernstyle.qss (@accent, @border, @buttonFill, ...), filled
    // in for the current theme, and they stay when the theme or the accent
    // changes. Calling it again replaces the previous rules.
    //   style->setExtraStyleSheet("QToolButton#fooName { border: 2px groove @border; }");
    QString extraStyleSheet() const { return m_extraSheet; }
    void setExtraStyleSheet(const QString &rules);

    // The style sheet ModernStyle put on the application, colours filled in.
    // To add rules for the whole application, append yours:
    //   qApp->setStyleSheet(style->currentStyleSheet() + myRules);
    // (re-do that after setTheme() / setAccentColor(), e.g. on themeChanged).
    QString currentStyleSheet() const { return m_appliedSheet; }

    // QStyle
    QPalette standardPalette() const override;
    void polish(QPalette &palette) override;
    void polish(QApplication *app) override;
    void polish(QWidget *widget) override;
    void unpolish(QWidget *widget) override;
    using QProxyStyle::polish;
    using QProxyStyle::unpolish;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    // After the application switched to the light or the dark theme
    void themeChanged(bool dark);

private:
    using Colors = QHash<QString, QColor>;
    Colors colors() const;           // the current theme's colours, by name
    QString styleSheet() const;      // modernstyle.qss with the colours filled in
    QString writeImages() const;     // check boxes, arrows, ...: their file name prefix
    void apply();                    // palette + style sheet onto the application

    Theme m_theme;
    QColor m_accent;
    QPalette m_systemPalette;        // the palette before this style changed it
    bool m_applying = false;
    int m_comboPopupMaxHeight = 0;
    QString m_extraSheet;            // setExtraStyleSheet()
    QString m_appliedSheet;          // the sheet apply() set on the application
    bool m_warnedSheetReplaced = false;
    QString m_checkedSheet;          // the last other application sheet looked at
    bool m_dockTitleBars = true;
    mutable std::unique_ptr<QTemporaryDir> m_imageDir;   // the drawn images
};
