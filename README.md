# Qt Frameless Window with Modern Style

A frameless Qt 6 window with its own title bar, rounded corners and a soft shadow, plus **ModernStyle**: a flat, rounded Qt Widgets style with a **light and a dark theme that follows the operating system**. Put your existing `QMainWindow` inside and it gets the modern look without other changes.

| Light | Dark |
|---|---|
| ![light](docs/screenshot-light.png) | ![dark](docs/screenshot-dark.png) |

Version 2 of this fork brings the original [Qt-Frameless-Window-DarkStyle](https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle) (Qt 5, qmake) to **Qt 6 and CMake**.

## Features

**FramelessWindow**
- Own title bar: icon, title, minimize / maximize (restore) / close buttons drawn in code, so they're sharp at any display scaling and follow the theme. The close button turns red on hover.
- **Native moving and resizing**: dragging the title bar or an edge hands over to the window system (`QWindow::startSystemMove()` / `startSystemResize()`). That gives native behaviour like snapping to screen edges, multi-monitor moves, and works on Wayland. Where the window system can't (macOS for resizing, some X11 window managers), the window moves or resizes itself.
- Resize from all four edges and corners, with the matching mouse cursors.
- Double-click the title bar to maximize or restore. Dragging a maximized window restores it under the mouse.
- Rounded corners and a shadow, none of either when maximized. Taskbar minimize and the window menu (Alt+Space) keep working on Windows.
- Takes its title and icon from the content (e.g. your `QMainWindow`) and follows later title changes.

**ModernStyle**
- **The palettes are the application's own dark and white palettes**, role for role (`Window` `rgb(53,53,53)` / white, `Base`, `AlternateBase`, `Highlight` `rgb(42,130,218)` / `rgb(90,154,209)`, the disabled colours, ...): `standardPalette()` returns exactly those, and the style sheet's colours are taken from them. Buttons are grey gradients on top, raised and darker when pressed.
- One accent colour, the palette's `Highlight` unless you set another, for selections, default buttons, focus, sliders, progress bars, check boxes and radio buttons.
- `Theme::System` (default) follows the OS light/dark setting and **switches live when it changes** (Qt 6.5+; older Qt picks at start-up).
- Switch at run time with `setTheme()`, change the accent with `setAccentColor()`.
- Themes by name, e.g. for a settings dialog or a command-line option: `getThemeNames()` gives `"System", "Light", "Dark"`, `ModernStyle::themeFromName("dark")` the theme (any case; `Theme::Invalid` for an unknown name), `ModernStyle::themeName(theme)` the name back.
- Checked buttons and tool buttons are blue (a `#428bca` gradient) and turn darker under the mouse; unchecked tool buttons light up under the mouse.
- The selected tab has a blue line under it. Group boxes are lighter, see-through boxes with the title as a small tab on top, underlined in grey: the application's own group box style sheet, pixel for pixel.
- Toolbar buttons are raised grey buttons with a groove edge, darker under the mouse and blue when checked; Qwt legend entries look the same (checked: bold and blue, unchecked: italic).
- Toolbars, the menu bar, the status bar and table/tree headers share a see-through band, darker towards the window edge; toolbars have a 3 px edge towards the contents (top, bottom, left and right toolbars). Menu bar items glow grey under the mouse and turn the accent colour when opened.
- The light theme uses the same colour replacements as the application's own light style (toolbar bands, toolbar buttons, tab and box fills, edges, the lighter blues of a hovered checked button). A toolbar too short for its buttons shows a 29 px wide `»` button drawn in the theme's colours.
- Group boxes and tab pages are lighter, see-through areas, so nested ones stand out.
- Menus: the accent colour on the item under the mouse, check boxes and radio buttons for checkable actions, a frame around the icon of a checked action. Tool tips have an accent border.
- Tree views draw branch lines and open/closed arrows. Check boxes and radio buttons have a pressed look.
- Splitter handles (10 px) and the gaps between dock widgets (6 px) have grip dots and turn `palette(highlight)` under the mouse; slider tracks and scroll bar handles are drawn in a colour that stands out on both the window and panels.
- Based on Qt's Fusion style, plus one readable style sheet (`modernstyle/modernstyle.qss`) with named colours (`@accent`, `@border`, ...) you can edit.
- Check boxes, radio buttons and arrows are drawn in the theme's colours at 1× and 2×: no image files, no Qt SVG module needed.
- **Dock widget icons**: `dock->setWindowIcon(icon)` shows the icon in the dock widget's title bar, on its tab when dock widgets are tabbed together, and in the title bar while it floats. ModernStyle gives dock widgets its own title bar (`DockTitleBar`: icon, title, float and close buttons) for this, because Qt's doesn't show icons, and a floating dock widget on Windows gets a system tool-window title bar without one. Dragging, double-click to float or dock again, and resizing a floating dock widget work as before. Dock widgets with a title bar widget of your own keep it; `style->setDockTitleBars(false)` turns it off for all (tabs still show icons).
- **Dock widgets are panels**: a body a little lighter than the window and a lighter title bar with rounded top corners and clear close / float buttons. Splitter handles, the gaps between dock widgets and the menu and toolbar separators use `palette(alternate-base)`.
- Toolbar buttons light up on hover like push buttons, checked ones included (a checked button gets brighter under the mouse).
- Long combo box lists scroll: `style->setComboPopupMaxHeight(240)` caps every drop-down at 240 px, and `combo->setProperty("popupMaxHeight", 120)` sets a limit for a single combo box. Without either, `QComboBox::maxVisibleItems()` (10) applies.
- The mouse wheel can leave combo boxes alone: `style->setComboWheelScrolling(false)` stops the wheel changing any combo box's value (it scrolls the panel or page under it instead), and `combo->setProperty("wheelScrolling", true / false)` decides for a single combo box. Clicking and the keyboard still change the value. On by default, as in Qt.
- `DarkStyle` still exists: `QApplication::setStyle(new DarkStyle)` keeps working (always dark).

## Requirements

- Qt **6.2** or newer (Widgets). Following the OS theme live needs Qt **6.5**.
- CMake 3.16+ and a C++17 compiler.
- Windows, Linux (X11 with a compositor, or Wayland), macOS.

## Build the demo

```bash
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.y/<kit>
cmake --build build
./build/QtFramelessWindowDemo                 # Windows: build\QtFramelessWindowDemo.exe
./build/QtFramelessWindowDemo --theme dark    # or light / system
```

The toolbar's **Theme** box switches between System, Light and Dark while it runs. `--screenshot file.png` saves a picture of the window and exits (that's how the screenshots above are made).

## Use it in your application

Add this repository to your project (a git submodule, a copy, or `FetchContent`) and link the library:

```cmake
add_subdirectory(third_party/Qt-Frameless-Window-DarkStyle)
target_link_libraries(MyApp PRIVATE FramelessWindow::FramelessWindow)
```

or without copying it:

```cmake
include(FetchContent)
FetchContent_Declare(FramelessWindow
    GIT_REPOSITORY https://github.com/cms26/Qt-Frameless-Window-DarkStyle.git
    GIT_TAG master)
FetchContent_MakeAvailable(FramelessWindow)
target_link_libraries(MyApp PRIVATE FramelessWindow::FramelessWindow)
```

The demo is only built when this is the top-level project (`FRAMELESSWINDOW_BUILD_DEMO`).

```cpp
#include "framelesswindow.h"
#include "modernstyle.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto *style = new ModernStyle;               // follows the OS; or ModernStyle(ModernStyle::Theme::Dark)
    QApplication::setStyle(style);               // the application takes ownership

    FramelessWindow window;
    window.setContent(new MainWindow);           // your QMainWindow (or any widget)
    window.resize(1000, 700);
    window.show();
    return app.exec();
}
```

At run time:

```cpp
style->setTheme(ModernStyle::Theme::Light);      // System, Light or Dark (Invalid is ignored)
themeBox->addItems(style->getThemeNames());      // "System", "Light", "Dark"
style->setTheme(ModernStyle::themeFromName(settings.value("theme").toString()));
style->setAccentColor(QColor("#7c3aed"));        // any colour; QColor() = the palette's Highlight again
connect(style, &ModernStyle::themeChanged, this, [](bool dark) { /* e.g. swap your own icons */ });

window.setResizable(false);                      // fixed size, no resize cursors
window.titleBar();                               // the title bar widget, e.g. to add widgets to it
```

You can also use only one of the two parts: `ModernStyle` works with ordinary windows, and `FramelessWindow` with any other style.

### Changing the look

The style sheet is [`modernstyle/modernstyle.qss`](modernstyle/modernstyle.qss). Colours are written as `@name` and filled in from the current theme. The names and their light/dark values are in `themeColors()` at the top of [`modernstyle/modernstyle.cpp`](modernstyle/modernstyle.cpp), made from the palettes in `themePalette()` (the palette role each comes from is noted). Buttons, tabs, bars and boxes use `@buttonFill`, `@tabFill`, `@boxFill` and friends (`themeFills()`), which can be gradients or see-through colours. Change values there, or add your own rules to the sheet; it is compiled in as a resource.

If your application sets its own style sheet with `qApp->setStyleSheet()`, it replaces ModernStyle's, and only the palette is left (ModernStyle then prints a warning). Set style sheets on your own widgets instead (`myWidget->setStyleSheet(...)`), or append your rules to ModernStyle's:

```cpp
style->setExtraStyleSheet(R"(
    QToolButton#fooName {
        padding: 5px 7px;
        background: @buttonFill;               /* ModernStyle's @names work here too */
        border: 2px groove @buttonBorder;
        border-radius: 4px;
    }
    QToolButton#fooName:hover   { background: @toolHoverFill; }
    QToolButton#fooName:checked { background: @checkedFill; color: @checkedText; }
)");
```

Your rules come after ModernStyle's, so they win where both style the same thing, and they stay when the theme or accent changes. The `@names` are filled in for the current theme, so one set of rules works in light and dark. A rule with an object name (`#fooName`) beats ModernStyle's `:hover`/`:checked` rules, so give it those states too, as above.

### Settings from Qt Designer

Frame settings made in Designer (or in code before the widget is shown) are followed, even on widgets ModernStyle draws its own border for:

| Setting | Result |
|---|---|
| `frameShape` = `NoFrame` (text boxes, lists, tables, trees, any frame) | no border |
| `frameShadow` = `Plain` (text boxes, lists, tables, trees) | a plain line, square corners |
| `frameShadow` = `Raised` | a raised edge |
| `frameShadow` = `Sunken` (their default) | ModernStyle's rounded border |
| `frame` off (`QLineEdit`, `QComboBox`, spin boxes) | no border |

Plain `QFrame`s ("Frame" in Designer: boxes, panels, lines) draw their shape and shadow as usual. If you change one of these settings on a widget that is already shown, call `w->style()->unpolish(w); w->style()->polish(w);` afterwards.

**Font sizes for single widgets**: give the widget a dynamic property `fontPointSize` (in Designer: the "+" above the property list, Number or Double, e.g. `14`), or set a size for an object name in code:

```cpp
label->setProperty("fontPointSize", 14);           // the same as the Designer property
style->setObjectFontPointSize("titleLabel", 18);   // every widget named "titleLabel", also ones created later
```

The widget's children use the size too, unless they have their own. It stays across theme changes. A style sheet rule with `font-size` for the same widget wins over both.

### If it doesn't look like the screenshots

- Check the program's output for `ModernStyle:` warnings: one says the style sheet resource is missing (link the `FramelessWindow::FramelessWindow` target instead of compiling the library's `.cpp` files into your own target), the other that `qApp->setStyleSheet()` replaced the sheet.
- Make sure no old `DarkStyle.cpp` / `darkstyle.qss` / `darkstyle.qrc` from the Qt 5 version is still in your project: its `DarkStyle` loads the old sheet over ModernStyle's. The library's `DarkStyle.h` is header-only.
- Don't set your own palette after `QApplication::setStyle()`; ModernStyle's palette is part of the look.

## Qwt plots

[Qwt](https://qwt.sourceforge.io) plots (6.2 or newer) get the same look in both themes:

![Qwt plot before and after, light and dark](docs/qwt-before-after.png)

Two parts do this:

- **The style sheet** gives the plot canvas a rounded frame and the theme's background, and the axes, title and legend the theme's text colours. It works for every `QwtPlot` automatically, with no Qwt dependency in this library.
- **`qwtmodernstyle.h`** (optional, header-only; include it where you use Qwt) styles what a style sheet can't reach: curve colours, the grid and the axes.

```cpp
#include "qwtmodernstyle.h"

QwtModernStyle::follow(plot, style);    // style the plot now and whenever the theme changes
```

It gives every curve a colour from a set that reads well on light and dark backgrounds, with 2 px antialiased lines. The grid becomes faint solid lines (one is added if the plot has none), and the axes lose their axis line and minor ticks. Leave parts out with options, e.g. `QwtModernStyle::follow(plot, style, QwtModernStyle::Option::All & ~QwtModernStyle::Option::CurveColors)` if your code colours its curves itself. `QwtModernStyle::seriesColor(index, dark)` gives you the colours for your own use (bars, markers). Call `apply()` again after adding curves.

Qwt caches what it draws, so a plot only shows a new theme after a replot. `follow()` does that for you. For plots you don't pass to it, call `plot->replot()` on `ModernStyle::themeChanged`.

The demo shows a **Plot** tab when CMake finds Qwt, e.g. winlibs' `qwt/6.2.0` on `CMAKE_PREFIX_PATH`. `--tab 2` opens it straight away.

## Upgrading from the Qt 5 version

| Before | Now |
|---|---|
| qmake: add `framelesswindow.qrc`, `darkstyle.qrc` and the include paths | CMake: `target_link_libraries(... FramelessWindow::FramelessWindow)` (resources and include paths come with it) |
| `#include "DarkStyle.h"`, `setStyle(new DarkStyle)` | unchanged (always dark), or `new ModernStyle` for light/dark |
| `FramelessWindow`, `setContent()`, `setWindowTitle()`, `setWindowIcon()` | unchanged |
| `darkstyle/darkstyle.qss` + 31 PNG icons | `modernstyle/modernstyle.qss`. Check boxes, radio buttons and arrows are drawn in code in the theme's colours; title-bar buttons are drawn by `TitleBarButton`; tree branches and dock-widget buttons come from Qt's Fusion style |
| `framelesswindow.ui`, `WindowDragger` | built in code (`TitleBar`, `TitleBarButton`) |

The old `ui->windowTitlebar` / `ui->windowFrame` objects no longer exist. Use `window.titleBar()` to reach the title bar.

## Platform notes

- **Windows**: moving, resizing, snapping by dragging to the screen edges, and minimize via the taskbar are native. Windows 11's snap layout flyout (hovering the maximize button) isn't offered: it needs Windows-specific code that answers the system's hit-testing for the maximize button.
- **Linux / X11**: rounded corners and the shadow need a compositing window manager (all common desktops have one); without one, the corners and shadow area are black.
- **Linux / Wayland**: moving and resizing only work through the compositor, which `startSystemMove()` / `startSystemResize()` use.
- **macOS**: there are no traffic-light buttons; the title bar uses the same buttons as on Windows and Linux. Resizing falls back to the window resizing itself.

## License

MIT, see [LICENSE](LICENSE). Based on Qt-Frameless-Window-DarkStyle by Juergen Skrotzky.
