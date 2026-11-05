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
    };
    Q_ENUM(Theme)

    explicit ModernStyle(Theme theme = Theme::System, QStyle *baseStyle = nullptr);
    ~ModernStyle() override;

    // The chosen theme, and whether that currently means dark
    Theme theme() const { return m_theme; }
    bool isDark() const;

    // Switch the theme of the running application
    void setTheme(Theme theme);

    // The colour of selections, default buttons, focus frames, sliders and
    // progress bars. Default: a blue that suits both themes.
    QColor accentColor() const { return m_accent; }
    void setAccentColor(const QColor &color);

    // QStyle
    QPalette standardPalette() const override;
    void polish(QPalette &palette) override;
    void polish(QApplication *app) override;
    void polish(QWidget *widget) override;
    using QProxyStyle::polish;

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
    mutable std::unique_ptr<QTemporaryDir> m_imageDir;   // the drawn images
};
