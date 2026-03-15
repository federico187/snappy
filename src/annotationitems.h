#ifndef ANNOTATIONITEMS_H
#define ANNOTATIONITEMS_H

#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QPen>
#include <QBrush>
#include <QFont>

// Base class for movable/resizable annotation items
class AnnotationItem
{
public:
    virtual ~AnnotationItem() = default;
    virtual void setColor(const QColor &color) = 0;
    virtual void setStrokeWidth(int width) = 0;
};

// Rectangle annotation
class RectAnnotation : public QGraphicsRectItem, public AnnotationItem
{
public:
    RectAnnotation(const QRectF &rect, QGraphicsItem *parent = nullptr);
    void setColor(const QColor &color) override;
    void setStrokeWidth(int width) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};

// Ellipse annotation
class EllipseAnnotation : public QGraphicsEllipseItem, public AnnotationItem
{
public:
    EllipseAnnotation(const QRectF &rect, QGraphicsItem *parent = nullptr);
    void setColor(const QColor &color) override;
    void setStrokeWidth(int width) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};

// Arrow annotation
class ArrowAnnotation : public QGraphicsLineItem, public AnnotationItem
{
public:
    ArrowAnnotation(const QLineF &line, QGraphicsItem *parent = nullptr);
    void setColor(const QColor &color) override;
    void setStrokeWidth(int width) override;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    QColor m_color;
    int m_strokeWidth;
};

// Text annotation
class TextAnnotation : public QGraphicsTextItem, public AnnotationItem
{
public:
    TextAnnotation(const QString &text, QGraphicsItem *parent = nullptr);
    void setColor(const QColor &color) override;
    void setStrokeWidth(int width) override;
    void setFontSize(int size);
    void startEditing();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};

// Blur region annotation
class BlurAnnotation : public QGraphicsRectItem
{
public:
    BlurAnnotation(const QRectF &rect, QGraphicsItem *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    void setSourcePixmap(const QPixmap &source);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    QPixmap m_sourcePixmap;
};

// Marker/Highlight annotation
class MarkerAnnotation : public QGraphicsRectItem, public AnnotationItem
{
public:
    MarkerAnnotation(const QRectF &rect, QGraphicsItem *parent = nullptr);
    void setColor(const QColor &color) override;
    void setStrokeWidth(int width) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};

// Numbered pin annotation (Shottr-style ①②③...)
class NumberedAnnotation : public QGraphicsItem, public AnnotationItem
{
public:
    NumberedAnnotation(int number, const QPointF &pos, QGraphicsItem *parent = nullptr);
    void setColor(const QColor &color) override;
    void setStrokeWidth(int width) override;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    int m_number;
    QColor m_color;
    double m_radius;
};

// Measurement annotation — persistent ruler line with px distance label
class MeasureAnnotation : public QGraphicsItem, public AnnotationItem
{
public:
    MeasureAnnotation(const QPointF &start, const QPointF &end, QGraphicsItem *parent = nullptr);
    void setColor(const QColor &color) override;
    void setStrokeWidth(int width) override;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    QPointF m_start, m_end;
    QColor m_color;
};

#endif // ANNOTATIONITEMS_H
