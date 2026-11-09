// MIT License, see LICENSE. Based on Qt-Frameless-Window-DarkStyle by Juergen
// Skrotzky (https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle).

#include "framelesswindow.h"

#include "titlebar.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QWindow>

namespace {
constexpr int kShadowSize = 12;   // transparent border around the window for its shadow
constexpr int kCornerRadius = 8;
constexpr int kResizeGrip = 6;    // how far inside the visible edge resizing still starts
constexpr int kCornerGrip = 16;   // corner zones: resize in two directions
} // namespace

FramelessWindow::FramelessWindow(QWidget *parent)
    : QWidget(parent)
    , m_titleBar(new TitleBar(this))
    , m_contentArea(new QWidget(this))
{
    // No system title bar or frame. The min/max hints keep the native
    // behaviour on Windows: minimize by clicking the taskbar button, the
    // window menu (Alt+Space), and the minimize/maximize animations.
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint
                   | Qt::WindowMinMaxButtonsHint);
    // Transparent outside the rounded frame, for the corners and the shadow
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);   // for the resize cursors at the edges

    m_contentArea->setObjectName(QStringLiteral("windowContent"));
    // The resize cursors are set on this window, at its edges, and child
    // widgets show their parent's cursor unless they have their own. The
    // mouse can go straight from an edge into the content, where this window
    // gets no more mouse moves to reset it, so the resize arrow would stay.
    // The content area (like the title bar) has its own normal arrow; widgets
    // inside it still set theirs (text cursor in text boxes, ...).
    m_contentArea->setCursor(Qt::ArrowCursor);
    auto *contentLayout = new QVBoxLayout(m_contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->addWidget(m_titleBar);
    layout->addWidget(m_contentArea, 1);

    connect(this, &QWidget::windowTitleChanged, m_titleBar, &TitleBar::setTitle);
    connect(this, &QWidget::windowIconChanged, m_titleBar, &TitleBar::setIcon);
    updateLayout();
}

void FramelessWindow::setContent(QWidget *content)
{
    if (m_content) {
        m_contentArea->layout()->removeWidget(m_content);
        m_content->deleteLater();
    }
    m_content = content;
    if (!content)
        return;
    m_contentArea->layout()->addWidget(content);

    // Use the content's title and icon if the window has none of its own
    if (windowTitle().isEmpty()) {
        QWidget::setWindowTitle(content->windowTitle());
        connect(content, &QWidget::windowTitleChanged, this, &FramelessWindow::setWindowTitle);
    }
    if (windowIcon().cacheKey() == QApplication::windowIcon().cacheKey() && !content->windowIcon().isNull())
        QWidget::setWindowIcon(content->windowIcon());
}

// --- Geometry ------------------------------------------------------------------

int FramelessWindow::shadowMargin() const
{
    return isMaximizedOrFullScreen() ? 0 : kShadowSize;
}

QRect FramelessWindow::frameRect() const
{
    const int m = shadowMargin();
    return rect().adjusted(m, m, -m, -m);
}

void FramelessWindow::updateLayout()
{
    // Inside the shadow, and 1 px in from the border line
    const int m = shadowMargin() + (isMaximizedOrFullScreen() ? 0 : 1);
    layout()->setContentsMargins(m, m, m, m);
    m_titleBar->updateState();
    update();
}

// --- Painting: shadow, rounded background, border ----------------------------------

void FramelessWindow::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF frame = QRectF(frameRect()).adjusted(0.5, 0.5, -0.5, -0.5);
    const bool rounded = !isMaximizedOrFullScreen();
    const qreal radius = rounded ? kCornerRadius : 0;

    if (rounded) {
        // Not quite transparent: on Windows, clicks on fully transparent
        // pixels go to the window behind, and the shadow border is where
        // the resize handles are
        p.fillRect(rect(), QColor(0, 0, 0, 1));

        // A soft shadow: rings around the frame, fading outwards, 1 px lower
        // than the window (light from above). Stronger while active.
        const int maxAlpha = isActiveWindow() ? 34 : 20;
        p.setBrush(Qt::NoBrush);
        for (int i = 1; i < kShadowSize; ++i) {
            const double fade = 1.0 - double(i) / kShadowSize;
            p.setPen(QPen(QColor(0, 0, 0, int(maxAlpha * fade * fade)), 1.0));
            p.drawRoundedRect(frame.adjusted(-i, -i + 1, i, i + 1), radius + i, radius + i);
        }
    }

    // The window itself: background and a thin border
    p.setPen(QPen(palette().color(QPalette::Midlight), 1.0));
    p.setBrush(palette().color(QPalette::Window));
    p.drawRoundedRect(frame, radius, radius);
}

// --- State changes -----------------------------------------------------------------

void FramelessWindow::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::WindowStateChange:      // maximized / restored: no shadow, square corners
    case QEvent::ActivationChange:       // active / inactive: title and shadow
    case QEvent::PaletteChange:          // light / dark theme
        updateLayout();
        break;
    default:
        break;
    }
    QWidget::changeEvent(event);
}

bool FramelessWindow::event(QEvent *event)
{
    // The cursor shape belongs to the edges only
    if (event->type() == QEvent::Leave && m_manualResizeEdges == Qt::Edges())
        unsetCursor();
    return QWidget::event(event);
}

// --- Resizing at the edges -------------------------------------------------------------
// The shadow border around the visible window belongs to this widget (not to
// the content), so the mouse events for resizing arrive here. When the
// window is maximized there's no border and no resizing.

Qt::Edges FramelessWindow::edgesAt(const QPoint &pos) const
{
    if (!m_resizable || isMaximizedOrFullScreen())
        return {};
    const QRect f = frameRect();
    Qt::Edges edges;
    if (pos.x() < f.left() + kResizeGrip)
        edges |= Qt::LeftEdge;
    if (pos.x() > f.right() - kResizeGrip)
        edges |= Qt::RightEdge;
    if (pos.y() < f.top() + kResizeGrip)
        edges |= Qt::TopEdge;
    if (pos.y() > f.bottom() - kResizeGrip)
        edges |= Qt::BottomEdge;

    // Near a corner, resize in both directions
    if (edges & (Qt::LeftEdge | Qt::RightEdge)) {
        if (pos.y() < f.top() + kCornerGrip)
            edges |= Qt::TopEdge;
        else if (pos.y() > f.bottom() - kCornerGrip)
            edges |= Qt::BottomEdge;
    }
    if (edges & (Qt::TopEdge | Qt::BottomEdge)) {
        if (pos.x() < f.left() + kCornerGrip)
            edges |= Qt::LeftEdge;
        else if (pos.x() > f.right() - kCornerGrip)
            edges |= Qt::RightEdge;
    }
    return edges;
}

static Qt::CursorShape cursorFor(Qt::Edges edges)
{
    const bool left = edges & Qt::LeftEdge, right = edges & Qt::RightEdge;
    const bool top = edges & Qt::TopEdge, bottom = edges & Qt::BottomEdge;
    if ((left && top) || (right && bottom))
        return Qt::SizeFDiagCursor;
    if ((right && top) || (left && bottom))
        return Qt::SizeBDiagCursor;
    if (left || right)
        return Qt::SizeHorCursor;
    return Qt::SizeVerCursor;
}

void FramelessWindow::mousePressEvent(QMouseEvent *event)
{
    const Qt::Edges edges = edgesAt(event->position().toPoint());
    if (event->button() == Qt::LeftButton && edges) {
        // Let the window system resize the window: native feel, and the
        // only way on Wayland
        if (windowHandle() && windowHandle()->startSystemResize(edges))
            return;
        // Not supported (macOS): resize it ourselves in mouseMoveEvent()
        m_manualResizeEdges = edges;
        m_pressGlobalPos = event->globalPosition().toPoint();
        m_startGeometry = geometry();
        return;
    }
    QWidget::mousePressEvent(event);
}

void FramelessWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_manualResizeEdges) {
        const QPoint delta = event->globalPosition().toPoint() - m_pressGlobalPos;
        QRect g = m_startGeometry;
        const QSize minimum = minimumSizeHint().expandedTo(minimumSize());
        if (m_manualResizeEdges & Qt::LeftEdge)
            g.setLeft(qMin(g.left() + delta.x(), g.right() - minimum.width()));
        if (m_manualResizeEdges & Qt::RightEdge)
            g.setRight(qMax(g.right() + delta.x(), g.left() + minimum.width()));
        if (m_manualResizeEdges & Qt::TopEdge)
            g.setTop(qMin(g.top() + delta.y(), g.bottom() - minimum.height()));
        if (m_manualResizeEdges & Qt::BottomEdge)
            g.setBottom(qMax(g.bottom() + delta.y(), g.top() + minimum.height()));
        setGeometry(g);
        return;
    }

    const Qt::Edges edges = edgesAt(event->position().toPoint());
    if (edges)
        setCursor(cursorFor(edges));
    else
        unsetCursor();
    QWidget::mouseMoveEvent(event);
}

void FramelessWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_manualResizeEdges = {};
    QWidget::mouseReleaseEvent(event);
}
