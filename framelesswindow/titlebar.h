// MIT License, see LICENSE. Based on Qt-Frameless-Window-DarkStyle by Juergen
// Skrotzky (https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle).

// The title bar of a FramelessWindow: icon, title and the minimize /
// maximize / close buttons. Drag it to move the window, double-click it to
// maximize or restore.

#pragma once

#include <QAbstractButton>
#include <QPoint>
#include <QWidget>

class QLabel;

// A minimize, maximize, restore or close button. It draws its symbol itself,
// in the palette's text colour, so it is sharp at any screen scaling and
// follows the light/dark theme.
class TitleBarButton : public QAbstractButton
{
    Q_OBJECT

public:
    enum class Kind { Minimize, Maximize, Restore, Close };

    explicit TitleBarButton(Kind kind, QWidget *parent = nullptr);

    Kind kind() const { return m_kind; }
    void setKind(Kind kind);

    QSize sizeHint() const override { return { 46, 32 }; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    Kind m_kind;
    bool m_hovered = false;
};

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *window);

    void setTitle(const QString &title);
    void setIcon(const QIcon &icon);

    // Show the window's state: maximized (restore button) and active (title
    // colour). Called by FramelessWindow when either changes.
    void updateState();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    void toggleMaximized();
    void startMove(const QPoint &globalPos);
    void restoreUnderMouse(const QPoint &globalPos);
    void placeUnderMouse(const QPoint &globalPos);

    QWidget *m_window;
    QLabel *m_icon;
    QLabel *m_title;
    TitleBarButton *m_minimize;
    TitleBarButton *m_maximize;
    TitleBarButton *m_close;

    // Dragging: the press position, and the offset for platforms that can't
    // move the window themselves (fallback)
    bool m_pressed = false;
    bool m_manualMove = false;
    bool m_moveWhenRestored = false;   // dragging a maximized window: move once restored
    QSize m_maximizedSize;
    double m_restoreRelativeX = 0.5;    // where the mouse was on the maximized title bar
    int m_restoreLocalY = 0;
    QPoint m_pressGlobalPos;
    QPoint m_manualMoveOffset;
};
