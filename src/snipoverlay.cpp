#include "snipoverlay.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QScreen>

SnipOverlay::SnipOverlay(const QPixmap &bg, QWidget *parent)
    : QWidget(parent), m_bg(bg), m_sel(false)
{
    // BypassWindowManagerHint = instant, no window animation
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
                   | Qt::BypassWindowManagerHint);
    // Opaque paint = no background erase = no white flash
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
    QScreen *s = QApplication::primaryScreen();
    if (s) setGeometry(s->geometry());
}

void SnipOverlay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawPixmap(0, 0, m_bg);
    if (!m_sel) return;

    QRect r = QRect(m_p1, m_p2).normalized();
    p.setPen(QPen(QColor("#6C5CE7"), 2));
    p.setBrush(Qt::NoBrush);
    p.drawRect(r);

    QString sz = QString("%1 x %2").arg(r.width()).arg(r.height());
    QFont f = p.font(); f.setPixelSize(12); f.setBold(true); p.setFont(f);
    QFontMetrics fm(f);
    QRect tr = fm.boundingRect(sz).adjusted(-6,-2,6,2);
    int lx = r.right() - tr.width();
    int ly = r.bottom() + 4;
    if (ly + tr.height() > height()) ly = r.top() - tr.height() - 4;
    if (lx < 0) lx = r.left();
    tr.moveTo(lx, ly);
    p.setPen(Qt::NoPen); p.setBrush(QColor("#6C5CE7"));
    p.drawRoundedRect(tr, 3, 3);
    p.setPen(Qt::white); p.drawText(tr, Qt::AlignCenter, sz);
}

void SnipOverlay::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_p1 = m_p2 = e->pos(); m_sel = true; update();
    }
}

void SnipOverlay::mouseMoveEvent(QMouseEvent *e)
{
    if (m_sel) { m_p2 = e->pos(); update(); }
}

void SnipOverlay::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && m_sel) {
        m_sel = false;
        QRect r = QRect(m_p1, m_p2).normalized();
        if (r.width() > 5 && r.height() > 5)
            emit regionSelected(r);
    }
}

void SnipOverlay::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) emit cancelled();
}
