// FramelessWindow - a top-level window without the system title bar and
// frame, with its own title bar, rounded corners and a soft shadow. Put any
// widget (e.g. your QMainWindow) inside it:
//
//   FramelessWindow window;
//   window.setContent(new MainWindow);
//   window.show();
//
// Moving (drag the title bar) and resizing (drag the edges) are done by the
// window system where it supports it (Windows, X11, Wayland; macOS for
// moving), so they behave natively, e.g. snapping to screen edges.
//
// MIT License, see LICENSE. Based on FramelessWindow by Juergen Skrotzky
// (https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle).

#pragma once

#include <QRect>
#include <QWidget>

class TitleBar;

class FramelessWindow : public QWidget
{
    Q_OBJECT

public:
    explicit FramelessWindow(QWidget *parent = nullptr);

    // The widget shown below the title bar. The window takes ownership. If
    // the window has no title or icon yet, it uses the content's (and follows
    // later changes of the content's title).
    void setContent(QWidget *content);
    QWidget *content() const { return m_content; }

    TitleBar *titleBar() const { return m_titleBar; }

    // Whether the window can be resized by dragging its edges (default: yes)
    void setResizable(bool resizable) { m_resizable = resizable; }
    bool isResizable() const { return m_resizable; }

public slots:
    // Same as QWidget's; kept as slots for compatibility with the original
    // FramelessWindow
    void setWindowTitle(const QString &title) { QWidget::setWindowTitle(title); }
    void setWindowIcon(const QIcon &icon) { QWidget::setWindowIcon(icon); }

protected:
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;
    bool event(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool isMaximizedOrFullScreen() const { return isMaximized() || isFullScreen(); }
    int shadowMargin() const;        // transparent border for the shadow (0 when maximized)
    QRect frameRect() const;         // the visible window, inside the shadow
    Qt::Edges edgesAt(const QPoint &pos) const;
    void updateLayout();

    TitleBar *m_titleBar;
    QWidget *m_contentArea;
    QWidget *m_content = nullptr;
    bool m_resizable = true;

    // Resizing ourselves, for platforms whose window system can't (macOS)
    Qt::Edges m_manualResizeEdges;
    QPoint m_pressGlobalPos;
    QRect m_startGeometry;
};
