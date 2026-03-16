#ifndef CANVASVIEW_H
#define CANVASVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsPathItem>
#include <QPixmap>
#include <QStack>

class CanvasView : public QGraphicsView
{
    Q_OBJECT
public:
    enum Tool {
        ToolNone = 0, ToolSelect, ToolRect, ToolEllipse, ToolArrow,
        ToolLine, ToolText, ToolBlur, ToolMeasure, ToolColorPick,
        ToolPen, ToolNumbered, ToolMagnifier, ToolCrop
    };

    explicit CanvasView(const QPixmap &screenshot, QWidget *parent = nullptr);

    void setCurrentTool(Tool t);
    Tool currentTool() const { return m_tool; }
    void setCurrentColor(const QColor &c);  // updates active/selected items (non-Select modes)
    void setDrawingColor(const QColor &c) { m_color = c; }  // future drawings only
    void applyColorToSelection(const QColor &c); // always applies to selected items
    QColor currentColor() const { return m_color; }
    void setStrokeWidth(int w);
    int  strokeWidth() const { return m_width; }

    QPixmap renderToPixmap() const;
    void deleteSelectedItems();
    void replaceScreenshot(const QPixmap &pm);

    void undo();
    void redo();
    void startCropMode();
    void zoomToFit();

    qreal currentZoom() const { return m_zoom; }

signals:
    void colorPicked(const QColor &c);
    void pixelInfoChanged(const QPoint &pos, const QColor &c);
    void zoomChanged(qreal z);
    void imageCropped(const QPixmap &img);
    void toolSwitchedToSelect();

protected:
    void wheelEvent(QWheelEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void drawForeground(QPainter *p, const QRectF &r) override;

private:
    void addAnnotation(const QPointF &a, const QPointF &b);
    QColor colorAt(const QPoint &vp) const;
    void record(QGraphicsItem *it);

    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_pxItem;
    QPixmap m_screenshot;

    Tool m_tool;
    QColor m_color;
    int m_width;
    bool m_drawing;
    QPointF m_a, m_b;
    qreal m_zoom;

    QGraphicsPathItem *m_livePen;
    QPainterPath m_penPath;
    bool m_cropping;
    QRectF m_cropR;

    // Measurement
    bool m_measuring;
    QPointF m_measureStart, m_measureEnd;

    QStack<QGraphicsItem*> m_undo, m_redo;
    QStack<QList<QGraphicsItem*>> m_deleteHistory; // batches of deleted items for undo
    int m_numCounter;
    QPoint m_mousePos;
    QPoint m_selectPressPos; // for single-click vs drag detection
    bool m_hadSelectionOnPress;
    QGraphicsItem *m_activeItem; // last created/editing item for real-time updates

    // Middle-button panning
    bool m_panning;
    QPoint m_panStart;

    // Crop undo — saves full state
    QPixmap m_preCropScreenshot;
    QList<QGraphicsItem*> m_preCropItems;
    QStack<QGraphicsItem*> m_preCropUndo;
    QStack<QList<QGraphicsItem*>> m_preCropDeleteHistory;

    // Crop redo — saves post-crop state
    QPixmap m_postCropScreenshot;
};

#endif
