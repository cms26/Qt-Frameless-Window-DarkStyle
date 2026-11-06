// DockTitleBar - the title bar ModernStyle gives dock widgets: the dock's
// icon (windowIcon()), its title, and float / close buttons.
//
// Qt draws a dock widget's own title bar without an icon, and a floating dock
// widget gets the operating system's tool-window title bar (Windows, macOS)
// which shows none either. This title bar shows the icon in both cases:
//
//   dock->setWindowIcon(QIcon(":/icons/layers.svg"));   // that's all
//
// ModernStyle installs it on every QDockWidget that has no title bar widget of
// its own (ModernStyle::setDockTitleBars(false) turns that off). It can also
// be used with other styles: DockTitleBar::install(dock).
//
// Dragging, double-click to float / dock again, and resizing a floating dock
// widget keep working: QDockWidget handles the mouse events this widget
// doesn't use. The style sheet styles it by object name (modernstyle.qss):
// #ModernDockTitle, #ModernDockTitleIcon, #ModernDockTitleText,
// #ModernDockFloatButton, #ModernDockCloseButton; the property "floating"
// tells whether the dock widget is floating.
//
// MIT License, see LICENSE.

#pragma once

#include <QWidget>

class QDockWidget;
class QLabel;
class QToolButton;

class DockTitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit DockTitleBar(QDockWidget *dock);

    // Give `dock` a DockTitleBar, unless it already has a title bar widget
    // (its own or a DockTitleBar). Returns the DockTitleBar, or nullptr.
    static DockTitleBar *install(QDockWidget *dock);

    QDockWidget *dockWidget() const { return m_dock; }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void updateIcon();
    void updateTitle();
    void updateButtons();

    QDockWidget *m_dock;
    QLabel *m_icon;
    QLabel *m_title;
    QToolButton *m_float;
    QToolButton *m_close;
};
