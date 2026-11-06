// MIT License, see LICENSE.

#include "docktitlebar.h"

#include <QDockWidget>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QToolButton>

DockTitleBar::DockTitleBar(QDockWidget *dock)
    : QWidget(dock)
    , m_dock(dock)
    , m_icon(new QLabel(this))
    , m_title(new QLabel(this))
    , m_float(new QToolButton(this))
    , m_close(new QToolButton(this))
{
    setObjectName(QStringLiteral("ModernDockTitle"));
    m_icon->setObjectName(QStringLiteral("ModernDockTitleIcon"));
    m_title->setObjectName(QStringLiteral("ModernDockTitleText"));
    m_float->setObjectName(QStringLiteral("ModernDockFloatButton"));
    m_close->setObjectName(QStringLiteral("ModernDockCloseButton"));

    // The title gives way when the dock widget is narrow
    m_title->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_title->setTextFormat(Qt::PlainText);

    for (QToolButton *button : { m_float, m_close }) {
        button->setAutoRaise(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setIconSize(QSize(12, 12));
    }
    m_float->setToolTip(tr("Float / dock"));
    m_close->setToolTip(tr("Close"));
    // The buttons' icons come from the style sheet (qproperty-icon), so they
    // follow the theme. Without ModernStyle: Qt's standard ones.
    m_float->setIcon(style()->standardIcon(QStyle::SP_TitleBarNormalButton, nullptr, this));
    m_close->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton, nullptr, this));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 4, 4);
    layout->setSpacing(6);
    layout->addWidget(m_icon);
    layout->addWidget(m_title, 1);
    layout->addWidget(m_float);
    layout->addWidget(m_close);

    connect(m_float, &QToolButton::clicked, dock, [dock] { dock->setFloating(!dock->isFloating()); });
    connect(m_close, &QToolButton::clicked, dock, &QDockWidget::close);
    connect(dock, &QDockWidget::windowTitleChanged, this, &DockTitleBar::updateTitle);
    connect(dock, &QDockWidget::featuresChanged, this, &DockTitleBar::updateButtons);
    connect(dock, &QDockWidget::topLevelChanged, this, [this](bool floating) {
        // A floating dock widget is a window of its own: fill it, including
        // the edge it is resized by (otherwise left unpainted)
        m_dock->setAutoFillBackground(floating);
        setProperty("floating", floating);
        // Re-apply the style sheet, so rules on [floating="true"] take effect
        style()->unpolish(this);
        style()->polish(this);
        update();
    });
    dock->installEventFilter(this);       // icon changes

    setProperty("floating", dock->isFloating());
    updateIcon();
    updateTitle();
    updateButtons();
}

DockTitleBar *DockTitleBar::install(QDockWidget *dock)
{
    if (!dock || dock->titleBarWidget())
        return nullptr;
    auto *bar = new DockTitleBar(dock);
    dock->setTitleBarWidget(bar);
    return bar;
}

bool DockTitleBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_dock && event->type() == QEvent::WindowIconChange)
        updateIcon();
    return QWidget::eventFilter(watched, event);
}

// A plain QWidget subclass only draws its style sheet background (and
// border) when it paints them itself
void DockTitleBar::paintEvent(QPaintEvent *)
{
    QStyleOption option;
    option.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
}

void DockTitleBar::updateIcon()
{
    // Only an icon set on the dock widget itself; windowIcon() would otherwise
    // return the application's icon
    const bool hasIcon = m_dock->testAttribute(Qt::WA_SetWindowIcon) && !m_dock->windowIcon().isNull();
    m_icon->setVisible(hasIcon);
    if (hasIcon) {
        const int size = style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, this);
        m_icon->setPixmap(m_dock->windowIcon().pixmap(QSize(size, size), devicePixelRatio()));
    }
}

void DockTitleBar::updateTitle()
{
    // Mnemonics ("&File") aren't shown here: drop single '&', keep "&&" as '&'
    QString title = m_dock->windowTitle();
    title.replace(QLatin1String("&&"), QString(QChar(0x1)));
    title.remove(QLatin1Char('&'));
    title.replace(QChar(0x1), QLatin1Char('&'));
    m_title->setText(title);
}

void DockTitleBar::updateButtons()
{
    m_float->setVisible(m_dock->features() & QDockWidget::DockWidgetFloatable);
    m_close->setVisible(m_dock->features() & QDockWidget::DockWidgetClosable);
}
