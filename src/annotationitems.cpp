#include "annotationitems.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsEffect>
#include <QCursor>
#include <QtMath>
#include <QTextCursor>
#include <cmath>

// ============================================================================
// RectAnnotation
// ============================================================================

RectAnnotation::RectAnnotation(const QRectF &rect, QGraphicsItem *parent)
    : QGraphicsRectItem(rect, parent)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemSendsGeometryChanges);
    setPen(QPen(QColor("#E74C3C"), 3));
    setBrush(Qt::NoBrush);
}

void RectAnnotation::setColor(const QColor &color)
{
    QPen p = pen();
    p.setColor(color);
    setPen(p);
}

void RectAnnotation::setStrokeWidth(int width)
{
    QPen p = pen();
    p.setWidth(width);
    setPen(p);
}

void RectAnnotation::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsRectItem::mousePressEvent(event);
}

void RectAnnotation::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsRectItem::mouseMoveEvent(event);
}

QVariant RectAnnotation::itemChange(GraphicsItemChange change, const QVariant &value)
{
    return QGraphicsRectItem::itemChange(change, value);
}

// ============================================================================
// EllipseAnnotation
// ============================================================================

EllipseAnnotation::EllipseAnnotation(const QRectF &rect, QGraphicsItem *parent)
    : QGraphicsEllipseItem(rect, parent)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemSendsGeometryChanges);
    setPen(QPen(QColor("#E74C3C"), 3));
    setBrush(Qt::NoBrush);
}

void EllipseAnnotation::setColor(const QColor &color)
{
    QPen p = pen();
    p.setColor(color);
    setPen(p);
}

void EllipseAnnotation::setStrokeWidth(int width)
{
    QPen p = pen();
    p.setWidth(width);
    setPen(p);
}

void EllipseAnnotation::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsEllipseItem::mousePressEvent(event);
}

void EllipseAnnotation::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsEllipseItem::mouseMoveEvent(event);
}

QVariant EllipseAnnotation::itemChange(GraphicsItemChange change, const QVariant &value)
{
    return QGraphicsEllipseItem::itemChange(change, value);
}

// ============================================================================
// ArrowAnnotation
// ============================================================================

ArrowAnnotation::ArrowAnnotation(const QLineF &line, QGraphicsItem *parent)
    : QGraphicsLineItem(line, parent)
    , m_color(QColor("#E74C3C"))
    , m_strokeWidth(3)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemSendsGeometryChanges);
    setPen(QPen(m_color, m_strokeWidth));
}

void ArrowAnnotation::setColor(const QColor &color)
{
    m_color = color;
    QPen p = pen();
    p.setColor(color);
    setPen(p);
    update();
}

void ArrowAnnotation::setStrokeWidth(int width)
{
    m_strokeWidth = width;
    QPen p = pen();
    p.setWidth(width);
    setPen(p);
    update();
}

QRectF ArrowAnnotation::boundingRect() const
{
    return QGraphicsLineItem::boundingRect().adjusted(-15, -15, 15, 15);
}

void ArrowAnnotation::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                            QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    QLineF l = line();
    if (l.length() < 2) return;

    QPen p(m_color, m_strokeWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(p);
    painter->setBrush(Qt::NoBrush);

    // Shaft — full line from p1 to p2
    painter->drawLine(l);

    // Arrowhead — two simple lines from the tip
    double angle = std::atan2(l.p2().y() - l.p1().y(), l.p2().x() - l.p1().x());
    double headLen = 10.0 + m_strokeWidth * 2.0;
    double spread = M_PI / 6.0; // 30°

    QPointF w1 = l.p2() - QPointF(std::cos(angle - spread) * headLen,
                                    std::sin(angle - spread) * headLen);
    QPointF w2 = l.p2() - QPointF(std::cos(angle + spread) * headLen,
                                    std::sin(angle + spread) * headLen);

    painter->drawLine(l.p2(), w1);
    painter->drawLine(l.p2(), w2);

    if (isSelected()) {
        painter->setPen(QPen(QColor(100, 100, 255, 128), 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

void ArrowAnnotation::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsLineItem::mousePressEvent(event);
}

void ArrowAnnotation::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsLineItem::mouseMoveEvent(event);
}

QVariant ArrowAnnotation::itemChange(GraphicsItemChange change, const QVariant &value)
{
    return QGraphicsLineItem::itemChange(change, value);
}

// ============================================================================
// TextAnnotation
// ============================================================================

TextAnnotation::TextAnnotation(const QString &text, QGraphicsItem *parent)
    : QGraphicsTextItem(text, parent)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemSendsGeometryChanges);
    setDefaultTextColor(QColor("#E74C3C"));
    QFont f = font();
    f.setPixelSize(18);
    f.setBold(true);
    setFont(f);
}

void TextAnnotation::setColor(const QColor &color)
{
    setDefaultTextColor(color);
}

void TextAnnotation::setStrokeWidth(int /*width*/)
{
    // Not applicable for text
}

void TextAnnotation::setFontSize(int size)
{
    QFont f = font();
    f.setPixelSize(size);
    setFont(f);
}

void TextAnnotation::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsTextItem::mousePressEvent(event);
}

void TextAnnotation::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsTextItem::mouseMoveEvent(event);
}

void TextAnnotation::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    setTextInteractionFlags(Qt::TextEditorInteraction);
    QGraphicsTextItem::mouseDoubleClickEvent(event);
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::Document);
    setTextCursor(cursor);
}

void TextAnnotation::startEditing()
{
    setTextInteractionFlags(Qt::TextEditorInteraction);
    setCursor(QCursor(Qt::IBeamCursor));
    setFocus();
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
}

QVariant TextAnnotation::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged && !value.toBool()) {
        // Clear any text selection before disabling interaction
        QTextCursor tc = textCursor();
        tc.clearSelection();
        setTextCursor(tc);
        setTextInteractionFlags(Qt::NoTextInteraction);
        setCursor(QCursor(Qt::ArrowCursor));
    }
    return QGraphicsTextItem::itemChange(change, value);
}

// ============================================================================
// BlurAnnotation
// ============================================================================

BlurAnnotation::BlurAnnotation(const QRectF &rect, QGraphicsItem *parent)
    : QGraphicsRectItem(rect, parent)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemSendsGeometryChanges);
    setPen(QPen(QColor(200, 200, 200, 128), 1, Qt::DashLine));
    setBrush(Qt::NoBrush);
}

void BlurAnnotation::setSourcePixmap(const QPixmap &source)
{
    m_sourcePixmap = source;
    update();
}

void BlurAnnotation::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                           QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QRectF r = rect();

    if (!m_sourcePixmap.isNull()) {
        // Source is already the exact region - just pixelate it
        int pixelSize = 10;
        QPixmap small = m_sourcePixmap.scaled(
            qMax(1, m_sourcePixmap.width() / pixelSize),
            qMax(1, m_sourcePixmap.height() / pixelSize),
            Qt::IgnoreAspectRatio, Qt::FastTransformation);
        QPixmap pixelated = small.scaled(
            m_sourcePixmap.width(), m_sourcePixmap.height(),
            Qt::IgnoreAspectRatio, Qt::FastTransformation);
        painter->drawPixmap(r, pixelated, pixelated.rect());
    } else {
        // Fallback: just draw a filled semi-transparent rect
        painter->fillRect(r, QColor(128, 128, 128, 180));
    }

    // Selection indicator
    if (isSelected()) {
        painter->setPen(QPen(QColor(100, 100, 255, 128), 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(r);
    }
}

void BlurAnnotation::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsRectItem::mousePressEvent(event);
}

void BlurAnnotation::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsRectItem::mouseMoveEvent(event);
}

QVariant BlurAnnotation::itemChange(GraphicsItemChange change, const QVariant &value)
{
    return QGraphicsRectItem::itemChange(change, value);
}

// ============================================================================
// MarkerAnnotation (Highlight)
// ============================================================================

MarkerAnnotation::MarkerAnnotation(const QRectF &rect, QGraphicsItem *parent)
    : QGraphicsRectItem(rect, parent)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemSendsGeometryChanges);
    setPen(Qt::NoPen);
    setBrush(QColor(255, 255, 0, 80)); // Semi-transparent yellow highlight
}

void MarkerAnnotation::setColor(const QColor &color)
{
    QColor c = color;
    c.setAlpha(80);
    setBrush(c);
}

void MarkerAnnotation::setStrokeWidth(int /*width*/)
{
    // Not really applicable, but could adjust height
}

void MarkerAnnotation::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsRectItem::mousePressEvent(event);
}

void MarkerAnnotation::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsRectItem::mouseMoveEvent(event);
}

QVariant MarkerAnnotation::itemChange(GraphicsItemChange change, const QVariant &value)
{
    return QGraphicsRectItem::itemChange(change, value);
}

// ============================================================================
// NumberedAnnotation
// ============================================================================

NumberedAnnotation::NumberedAnnotation(int number, const QPointF &pos, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_number(number)
    , m_color(QColor("#E74C3C"))
    , m_radius(14.0)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemSendsGeometryChanges);
    setPos(pos);
    setZValue(200);
}

void NumberedAnnotation::setColor(const QColor &color)
{
    m_color = color;
    update();
}

void NumberedAnnotation::setStrokeWidth(int width)
{
    prepareGeometryChange();
    m_radius = qMax(10.0, width * 3.0);
    update();
}

QRectF NumberedAnnotation::boundingRect() const
{
    return QRectF(-m_radius - 2, -m_radius - 2, (m_radius + 2) * 2, (m_radius + 2) * 2);
}

void NumberedAnnotation::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
                                QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 60));
    painter->drawEllipse(QPointF(1.5, 1.5), m_radius, m_radius);

    painter->setBrush(m_color);
    painter->setPen(QPen(Qt::white, 2));
    painter->drawEllipse(QPointF(0, 0), m_radius, m_radius);

    painter->setPen(Qt::white);
    QFont font;
    font.setPixelSize(qRound(m_radius * 0.8));
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(m_number));

    if (isSelected()) {
        painter->setPen(QPen(QColor(100, 100, 255, 160), 1.5, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QPointF(0, 0), m_radius + 3, m_radius + 3);
    }
}

void NumberedAnnotation::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
}

void NumberedAnnotation::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);
}

QVariant NumberedAnnotation::itemChange(GraphicsItemChange change, const QVariant &value)
{
    return QGraphicsItem::itemChange(change, value);
}

// ============================================================================
// MeasureAnnotation
// ============================================================================

MeasureAnnotation::MeasureAnnotation(const QPointF &start, const QPointF &end, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_start(start)
    , m_end(end)
    , m_color(QColor("#6C5CE7"))
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemSendsGeometryChanges);
    setZValue(200);
}

void MeasureAnnotation::setColor(const QColor &color)
{
    m_color = color;
    update();
}

void MeasureAnnotation::setStrokeWidth(int /*width*/)
{
    // Not applicable
}

QRectF MeasureAnnotation::boundingRect() const
{
    QRectF r = QRectF(m_start, m_end).normalized();
    return r.adjusted(-20, -25, 20, 10);
}

void MeasureAnnotation::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);

    double dx = m_end.x() - m_start.x();
    double dy = m_end.y() - m_start.y();
    double dist = std::sqrt(dx * dx + dy * dy);

    // Dashed line
    painter->setPen(QPen(m_color, 2, Qt::DashDotLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(m_start, m_end);

    // Endpoints
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_color);
    painter->drawEllipse(m_start, 4.0, 4.0);
    painter->drawEllipse(m_end, 4.0, 4.0);

    // Distance label at midpoint
    QPointF mid = (m_start + m_end) / 2.0;
    QString label = QString("%1 px").arg(qRound(dist));
    QFont f;
    f.setPixelSize(12);
    f.setBold(true);
    painter->setFont(f);
    QFontMetrics fm(f);
    QRectF tr = fm.boundingRect(label);
    tr.adjust(-5, -2, 5, 2);
    tr.moveCenter(mid + QPointF(0, -12));
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_color);
    painter->drawRoundedRect(tr, 3, 3);
    painter->setPen(Qt::white);
    painter->drawText(tr, Qt::AlignCenter, label);

    // Selection highlight
    if (isSelected()) {
        painter->setPen(QPen(QColor(100, 100, 255, 160), 1.5, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

void MeasureAnnotation::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
}

void MeasureAnnotation::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);
}

QVariant MeasureAnnotation::itemChange(GraphicsItemChange change, const QVariant &value)
{
    return QGraphicsItem::itemChange(change, value);
}
