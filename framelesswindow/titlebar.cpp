// MIT License, see LICENSE. Based on Qt-Frameless-Window-DarkStyle by Juergen
// Skrotzky (https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle).

#include "titlebar.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QWindow>

// =============================================================================
// TitleBarButton
// =============================================================================

TitleBarButton::TitleBarButton(Kind kind, QWidget *parent)
    : QAbstractButton(parent)
    , m_kind(kind)
{
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::ArrowCursor);
}

void TitleBarButton::setKind(Kind kind)
{
    m_kind = kind;
    update();
}

void TitleBarButton::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    update();
    QAbstractButton::enterEvent(event);
}

void TitleBarButton::leaveEvent(QEvent *event)
{
    m_hovered = false;
    update();
    QAbstractButton::leaveEvent(event);
}

void TitleBarButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Background on hover / press: red for close (as on Windows), a light
    // tint of the text colour for the others
    QColor symbol = palette().color(QPalette::WindowText);
    if (!window()->isActiveWindow())
        symbol.setAlphaF(0.55);
    if (m_hovered || isDown()) {
        QColor background;
        if (m_kind == Kind::Close) {
            background = isDown() ? QColor("#a52a1d") : QColor("#c42b1c");
            symbol = Qt::white;
        } else {
            background = palette().color(QPalette::WindowText);
            background.setAlphaF(isDown() ? 0.16 : 0.09);
        }
        p.fillRect(rect(), background);
    }

    // The symbol: 10 x 10 device-independent pixels, centred
    QPen pen(symbol, 1.0);
    pen.setCapStyle(Qt::FlatCap);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    const QRectF box(width() / 2.0 - 5, height() / 2.0 - 5, 10, 10);

    switch (m_kind) {
    case Kind::Minimize:
        p.drawLine(QPointF(box.left(), box.center().y()), QPointF(box.right(), box.center().y()));
        break;
    case Kind::Maximize:
        p.drawRoundedRect(box.adjusted(0.5, 0.5, -0.5, -0.5), 1.5, 1.5);
        break;
    case Kind::Restore: {
        // A front window, and the top-right edge of one behind it
        p.drawRoundedRect(box.adjusted(0.5, 2.5, -2.5, -0.5), 1.5, 1.5);
        const QPointF back[] = {
            { box.left() + 2.5, box.top() + 2.5 },    // down from here is hidden by the front one
            { box.left() + 2.5, box.top() + 0.5 },
            { box.right() - 0.5, box.top() + 0.5 },
            { box.right() - 0.5, box.bottom() - 2.5 },
            { box.right() - 2.5, box.bottom() - 2.5 },
        };
        p.drawPolyline(back, 5);
        break;
    }
    case Kind::Close:
        p.drawLine(box.topLeft(), box.bottomRight());
        p.drawLine(box.topRight(), box.bottomLeft());
        break;
    }
}

// =============================================================================
// TitleBar
// =============================================================================

TitleBar::TitleBar(QWidget *window)
    : QWidget(window)
    , m_window(window)
    , m_icon(new QLabel(this))
    , m_title(new QLabel(this))
    , m_minimize(new TitleBarButton(TitleBarButton::Kind::Minimize, this))
    , m_maximize(new TitleBarButton(TitleBarButton::Kind::Maximize, this))
    , m_close(new TitleBarButton(TitleBarButton::Kind::Close, this))
{
    setObjectName(QStringLiteral("titleBar"));
    m_title->setObjectName(QStringLiteral("titleText"));
    m_icon->setFixedSize(16, 16);
    m_title->setTextInteractionFlags(Qt::NoTextInteraction);
    // Clicks on the icon and title go to the title bar (moving the window)
    m_icon->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_title->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_minimize->setToolTip(tr("Minimize"));
    m_maximize->setToolTip(tr("Maximize"));
    m_close->setToolTip(tr("Close"));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_icon);
    layout->addSpacing(10);
    layout->addWidget(m_title, 1);
    layout->addWidget(m_minimize);
    layout->addWidget(m_maximize);
    layout->addWidget(m_close);
    setFixedHeight(m_close->sizeHint().height());

    connect(m_minimize, &QAbstractButton::clicked, m_window, &QWidget::showMinimized);
    connect(m_maximize, &QAbstractButton::clicked, this, &TitleBar::toggleMaximized);
    connect(m_close, &QAbstractButton::clicked, m_window, &QWidget::close);

    updateState();
}

void TitleBar::setTitle(const QString &title)
{
    m_title->setText(title);
}

void TitleBar::setIcon(const QIcon &icon)
{
    m_icon->setPixmap(icon.pixmap(QSize(16, 16), devicePixelRatioF()));
    m_icon->setVisible(!icon.isNull());
}

void TitleBar::updateState()
{
    const bool maximized = m_window->isMaximized() || m_window->isFullScreen();
    m_maximize->setKind(maximized ? TitleBarButton::Kind::Restore : TitleBarButton::Kind::Maximize);
    m_maximize->setToolTip(maximized ? tr("Restore") : tr("Maximize"));

    // The style sheet dims the title of an inactive window
    // (TitleBar[active="false"]); a changed property needs a re-polish
    const bool active = m_window->isActiveWindow();
    if (property("active").toBool() != active || !property("active").isValid()) {
        setProperty("active", active);
        style()->unpolish(m_title);
        style()->polish(m_title);
    }
    update();
    for (QWidget *button : { static_cast<QWidget *>(m_minimize), static_cast<QWidget *>(m_maximize),
                             static_cast<QWidget *>(m_close) })
        button->update();
}

void TitleBar::toggleMaximized()
{
    if (m_window->isMaximized())
        m_window->showNormal();
    else
        m_window->showMaximized();
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        m_pressGlobalPos = event->globalPosition().toPoint();
    }
    QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint globalPos = event->globalPosition().toPoint();
    if (m_manualMove) {
        m_window->move(globalPos - m_manualMoveOffset);
    } else if (m_moveWhenRestored) {
        // Waiting for the window system to restore the window (below). Qt
        // reports "not maximized" as soon as it asked; the size changing
        // shows it really happened. The button is still down, so the drag
        // simply carries on from here.
        if (!m_window->isMaximized() && m_window->size() != m_maximizedSize) {
            // Finish this drag ourselves: handing it to the window system in
            // the same moment as placing the window races on X11
            m_moveWhenRestored = false;
            placeUnderMouse(globalPos);
            m_manualMove = true;
            m_manualMoveOffset = globalPos - m_window->pos();
        }
    } else if (m_pressed
               && (globalPos - m_pressGlobalPos).manhattanLength() >= QApplication::startDragDistance()) {
        // Start moving only once the mouse really moves, so a double-click
        // still reaches mouseDoubleClickEvent()
        m_pressed = false;
        if (m_window->isMaximized()) {
            restoreUnderMouse(globalPos);
            m_moveWhenRestored = true;
        } else {
            startMove(globalPos);
        }
    }
    QWidget::mouseMoveEvent(event);
}

void TitleBar::restoreUnderMouse(const QPoint &globalPos)
{
    // Dragging a maximized window restores it under the mouse, at the same
    // relative position along the title bar. The window system restores it
    // a moment later (X11), and moving or resizing it before then would
    // cancel the restore, so remember where the mouse was on the title bar,
    // and place the window in mouseMoveEvent() once it's restored. The rest
    // of that drag is moved by us, not the window system.
    const QPoint local = mapFromGlobal(globalPos);
    m_restoreRelativeX = double(local.x()) / qMax(1, width());
    m_restoreLocalY = local.y();
    m_maximizedSize = m_window->size();
    m_window->showNormal();
}

void TitleBar::placeUnderMouse(const QPoint &globalPos)
{
    const QMargins margins = m_window->layout() ? m_window->layout()->contentsMargins() : QMargins();
    const int innerWidth = m_window->width() - margins.left() - margins.right();
    m_window->move(globalPos.x() - margins.left() - int(m_restoreRelativeX * innerWidth),
                   globalPos.y() - margins.top() - m_restoreLocalY);
    m_pressGlobalPos = globalPos;    // this point of the window is now under the mouse
}

void TitleBar::startMove(const QPoint &globalPos)
{
    // Let the window system move the window: this gives native behaviour
    // such as snapping to screen edges, and works on Wayland
    if (QWindow *handle = m_window->windowHandle(); handle && handle->startSystemMove())
        return;

    // Not supported here (e.g. some X11 window managers): move it ourselves,
    // keeping the point that was pressed under the mouse
    m_manualMove = true;
    m_manualMoveOffset = m_pressGlobalPos - m_window->pos();
    m_window->move(globalPos - m_manualMoveOffset);
}

void TitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;
    m_manualMove = false;
    m_moveWhenRestored = false;
    QWidget::mouseReleaseEvent(event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        toggleMaximized();
    QWidget::mouseDoubleClickEvent(event);
}
