#include "canvasview.h"
#include "annotationitems.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QtMath>
#include <cmath>
#include <QScrollBar>
#include <QTimer>
#include <QTextCursor>

CanvasView::CanvasView(const QPixmap &screenshot, QWidget *parent)
    : QGraphicsView(parent)
    , m_scene(new QGraphicsScene(this))
    , m_screenshot(screenshot)
    , m_tool(ToolSelect)
    , m_color("#E74C3C")
    , m_width(3)
    , m_drawing(false)
    , m_zoom(1.0)
    , m_livePen(nullptr)
    , m_cropping(false)
    , m_numCounter(1)
    , m_panning(false)
    , m_measuring(false)
    , m_activeItem(nullptr)
{
    setScene(m_scene);
    setTransformationAnchor(AnchorUnderMouse);
    setResizeAnchor(AnchorUnderMouse);
    setBackgroundBrush(QColor("#1E1E22"));
    setDragMode(RubberBandDrag);

    // --- Memory optimizations ---
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, false);
    setViewportUpdateMode(MinimalViewportUpdate);
    setOptimizationFlag(DontSavePainterState, true);
    setOptimizationFlag(DontAdjustForAntialiasing, true);
    // NO CacheBackground — creates a full off-screen buffer
    // NO DeviceCoordinateCache — duplicates the entire pixmap at device DPI
    setMouseTracking(false);

    m_pxItem = m_scene->addPixmap(m_screenshot);
    m_pxItem->setZValue(0);

    QRectF sr = m_pxItem->boundingRect().adjusted(-20, -20, 20, 20);
    m_scene->setSceneRect(sr);

    setStyleSheet(
        "QGraphicsView{border:none;}"
        "QScrollBar:vertical,QScrollBar:horizontal{background:transparent;width:8px;height:8px;}"
        "QScrollBar::handle:vertical,QScrollBar::handle:horizontal{"
        "background:rgba(255,255,255,0.18);border-radius:4px;min-height:20px;}"
        "QScrollBar::add-line,QScrollBar::sub-line,QScrollBar::add-page,QScrollBar::sub-page{"
        "background:none;height:0;width:0;}"
    );
}

void CanvasView::setCurrentTool(Tool t)
{
    Tool oldTool = m_tool;
    m_tool = t;
    m_drawing = false;
    m_cropping = false;
    m_measuring = false;

    if (t == ToolSelect) {
        setDragMode(RubberBandDrag);
        viewport()->setCursor(Qt::ArrowCursor);
        setMouseTracking(true);
    } else if (t == ToolText) {
        setDragMode(NoDrag);
        setCursor(Qt::IBeamCursor);
        viewport()->setCursor(Qt::IBeamCursor);
        setMouseTracking(true);
    } else {
        setDragMode(NoDrag);
        setCursor(Qt::CrossCursor);
        viewport()->setCursor(Qt::CrossCursor);
        setMouseTracking(t == ToolColorPick || t == ToolMagnifier || t == ToolMeasure);
    }

    // Force repaint to clear stale foreground overlays (magnifier, measure preview)
    if (oldTool == ToolMagnifier || oldTool == ToolMeasure || oldTool != t)
        viewport()->update();
}

void CanvasView::setCurrentColor(const QColor &c)
{
    m_color = c;

    // In Select mode, only set future drawing color — don't touch existing items
    if (m_tool == ToolSelect || m_tool == ToolColorPick) return;

    // Helper: update color on any item type
    auto applyColor = [&](QGraphicsItem *it) {
        if (!it || it == m_pxItem) return;
        auto *ann = dynamic_cast<AnnotationItem*>(it);
        if (ann) { ann->setColor(c); return; }
        auto *li = dynamic_cast<QGraphicsLineItem*>(it);
        if (li) { QPen p = li->pen(); p.setColor(c); li->setPen(p); return; }
        auto *pi = dynamic_cast<QGraphicsPathItem*>(it);
        if (pi) { QPen p = pi->pen(); p.setColor(c); pi->setPen(p); return; }
    };

    // Update the active (last-created) item
    if (m_activeItem) applyColor(m_activeItem);

    // Update all selected items
    for (auto *it : m_scene->selectedItems()) applyColor(it);
}

void CanvasView::applyColorToSelection(const QColor &c)
{
    m_color = c;

    auto applyColor = [&](QGraphicsItem *it) {
        if (!it || it == m_pxItem) return;
        auto *ann = dynamic_cast<AnnotationItem*>(it);
        if (ann) { ann->setColor(c); return; }
        auto *li = dynamic_cast<QGraphicsLineItem*>(it);
        if (li) { QPen p = li->pen(); p.setColor(c); li->setPen(p); return; }
        auto *pi = dynamic_cast<QGraphicsPathItem*>(it);
        if (pi) { QPen p = pi->pen(); p.setColor(c); pi->setPen(p); return; }
    };

    if (m_activeItem) applyColor(m_activeItem);
    for (auto *it : m_scene->selectedItems()) applyColor(it);
}

void CanvasView::setStrokeWidth(int w)
{
    m_width = w;

    // In Select mode, only set future drawing width
    if (m_tool == ToolSelect || m_tool == ToolColorPick) return;

    // Helper: update width/size on any item type
    auto applyWidth = [&](QGraphicsItem *it) {
        if (!it || it == m_pxItem) return;
        // TextAnnotation uses width as font size
        auto *ta = dynamic_cast<TextAnnotation*>(it);
        if (ta) { ta->setFontSize(qMax(12, w * 4)); return; }
        // AnnotationItem subclasses (Rect, Ellipse, Arrow, Numbered)
        auto *ann = dynamic_cast<AnnotationItem*>(it);
        if (ann) { ann->setStrokeWidth(w); return; }
        // QGraphicsLineItem
        auto *li = dynamic_cast<QGraphicsLineItem*>(it);
        if (li) { QPen p = li->pen(); p.setWidth(w); li->setPen(p); return; }
        // QGraphicsPathItem (freehand)
        auto *pi = dynamic_cast<QGraphicsPathItem*>(it);
        if (pi) { QPen p = pi->pen(); p.setWidth(w); pi->setPen(p); return; }
    };

    // Update the active (last-created) item
    if (m_activeItem) applyWidth(m_activeItem);

    // Update all selected items
    for (auto *it : m_scene->selectedItems()) applyWidth(it);
}

void CanvasView::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        double f = e->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
        m_zoom *= f;
        m_zoom = qBound(0.05, m_zoom, 50.0);
        scale(f, f);
        emit zoomChanged(m_zoom);
        e->accept();
    } else {
        QGraphicsView::wheelEvent(e);
    }
}

void CanvasView::mousePressEvent(QMouseEvent *e)
{
    // Middle button = pan
    if (e->button() == Qt::MiddleButton) {
        m_panning = true;
        m_panStart = e->pos();
        setCursor(Qt::ClosedHandCursor);
        e->accept();
        return;
    }

    // Ctrl+Left click = pan
    if (e->button() == Qt::LeftButton && (e->modifiers() & Qt::ControlModifier)) {
        m_panning = true;
        m_panStart = e->pos();
        setCursor(Qt::ClosedHandCursor);
        e->accept();
        return;
    }

    // Ctrl+Left = pan
    if (e->button() == Qt::LeftButton && (e->modifiers() & Qt::ControlModifier)) {
        m_panning = true;
        m_panStart = e->pos();
        setCursor(Qt::ClosedHandCursor);
        e->accept();
        return;
    }

    if (e->button() != Qt::LeftButton) { QGraphicsView::mousePressEvent(e); return; }
    QPointF sp = mapToScene(e->pos());
    m_mousePos = e->pos();

    // If a text item is being edited and user clicks elsewhere
    if (m_scene->focusItem()) {
        auto *ti = dynamic_cast<QGraphicsTextItem*>(m_scene->focusItem());
        if (ti && (ti->textInteractionFlags() & Qt::TextEditorInteraction)) {
            QGraphicsItem *clickedItem = m_scene->itemAt(sp, transform());
            if (clickedItem == ti) {
                // Clicking on the same text item — let Qt handle (double-click selects word)
                QGraphicsView::mousePressEvent(e);
                return;
            }

            // Finish editing current text — clear selection highlight
            auto *oldTa = dynamic_cast<TextAnnotation*>(ti);
            if (oldTa) {
                QTextCursor tc = oldTa->textCursor();
                tc.clearSelection();
                oldTa->setTextCursor(tc);
            }
            ti->setTextInteractionFlags(Qt::NoTextInteraction);
            ti->clearFocus();

            // If in Text mode and clicked on ANOTHER text, transfer editing to it
            if (m_tool == ToolText) {
                auto *newTa = dynamic_cast<TextAnnotation*>(clickedItem);
                if (newTa) {
                    newTa->startEditing();
                    m_activeItem = newTa;
                    return;
                }
                // Clicked on empty area in Text mode — stay in Text mode
                // (will fall through to default: case which starts drawing)
            } else {
                // Not in Text mode — switch to Select
                m_activeItem = nullptr;
                setCurrentTool(ToolSelect);
                emit toolSwitchedToSelect();
            }
            QGraphicsView::mousePressEvent(e);
            return;
        }
    }

    switch (m_tool) {
    case ToolSelect: {
        QGraphicsItem *clicked = m_scene->itemAt(sp, transform());
        bool clickedBackground = (!clicked || clicked == m_pxItem);
        bool hadSelection = !m_scene->selectedItems().isEmpty();

        // Pick color ONLY when clicking empty background with nothing selected
        if (clickedBackground && !hadSelection) {
            QColor c = colorAt(e->pos());
            if (c.isValid()) {
                m_color = c;
                emit colorPicked(c);
            }
        }

        QGraphicsView::mousePressEvent(e);
        return;
    }
    case ToolColorPick:
        emit colorPicked(colorAt(e->pos()));
        return;
    case ToolMagnifier: {
        // Magnifier also picks color on click
        QColor c = colorAt(e->pos());
        if (c.isValid()) {
            m_color = c;
            emit colorPicked(c);
            emit pixelInfoChanged(QPoint(qRound(sp.x()), qRound(sp.y())), c);
        }
        return;
    }
    case ToolMeasure:
        m_measuring = true;
        m_measureStart = m_measureEnd = sp;
        setMouseTracking(true);
        return;
    case ToolCrop:
        m_cropping = true;
        m_cropR = QRectF(sp, sp);
        return;
    case ToolNumbered: {
        auto *n = new NumberedAnnotation(m_numCounter++, sp);
        n->setColor(m_color);
        n->setStrokeWidth(m_width);
        m_scene->addItem(n);
        record(n);
        m_activeItem = n;
        return;
    }
    case ToolPen:
        m_penPath = QPainterPath();
        m_penPath.moveTo(sp);
        m_livePen = new QGraphicsPathItem(m_penPath);
        m_livePen->setPen(QPen(m_color, m_width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        m_livePen->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
        m_livePen->setZValue(100);
        m_scene->addItem(m_livePen);
        m_drawing = true;
        return;
    default:
        // Text tool: if clicking on an existing TextAnnotation, edit it
        if (m_tool == ToolText) {
            QGraphicsItem *clicked = m_scene->itemAt(sp, transform());
            auto *ta = dynamic_cast<TextAnnotation*>(clicked);
            if (ta) {
                // Just start editing — don't change its color or size
                ta->startEditing();
                m_activeItem = ta;
                return;
            }
        }
        m_drawing = true;
        m_a = m_b = sp;
        return;
    }
}

void CanvasView::mouseMoveEvent(QMouseEvent *e)
{
    // Middle-button panning
    if (m_panning) {
        QPoint delta = e->pos() - m_panStart;
        m_panStart = e->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        e->accept();
        return;
    }

    QPointF sp = mapToScene(e->pos());
    m_mousePos = e->pos();

    // Emit color info for Select and ColorPick modes
    if (m_tool == ToolSelect || m_tool == ToolColorPick) {
        QColor c = colorAt(e->pos());
        if (c.isValid())
            emit pixelInfoChanged(QPoint(qRound(sp.x()), qRound(sp.y())), c);
    }

    if (m_tool == ToolSelect) { QGraphicsView::mouseMoveEvent(e); return; }

    // Text tool cursor logic
    if (m_tool == ToolText && !m_drawing) {
        bool isEditing = false;
        if (m_scene->focusItem()) {
            auto *fi = dynamic_cast<QGraphicsTextItem*>(m_scene->focusItem());
            if (fi && (fi->textInteractionFlags() & Qt::TextEditorInteraction))
                isEditing = true;
        }
        if (isEditing) {
            // While editing: arrow cursor outside the text box
            QGraphicsItem *hovered = m_scene->itemAt(sp, transform());
            auto *ta = dynamic_cast<TextAnnotation*>(hovered);
            viewport()->setCursor(ta ? Qt::IBeamCursor : Qt::ArrowCursor);
        } else {
            // Not editing: IBeam over existing text, IBeam elsewhere (ready to create)
            viewport()->setCursor(Qt::IBeamCursor);
        }
    }

    if (m_measuring) { m_measureEnd = sp; viewport()->update(); return; }
    if (m_cropping) { m_cropR.setBottomRight(sp); viewport()->update(); return; }
    if (m_drawing && m_tool == ToolPen && m_livePen) {
        m_penPath.lineTo(sp);
        m_livePen->setPath(m_penPath);
        return;
    }
    if (m_drawing) { m_b = sp; viewport()->update(); return; }
    if (m_tool == ToolMagnifier) {
        QColor c = colorAt(e->pos());
        if (c.isValid())
            emit pixelInfoChanged(QPoint(qRound(sp.x()), qRound(sp.y())), c);
        viewport()->update();
        return;
    }
    QGraphicsView::mouseMoveEvent(e);
}

void CanvasView::mouseReleaseEvent(QMouseEvent *e)
{
    // Pan release (middle or ctrl+left)
    if (m_panning && (e->button() == Qt::MiddleButton || e->button() == Qt::LeftButton)) {
        m_panning = false;
        Qt::CursorShape c = m_tool == ToolSelect ? Qt::ArrowCursor : Qt::CrossCursor;
        setCursor(c);
        viewport()->setCursor(c);
        e->accept();
        return;
    }

    if (e->button() != Qt::LeftButton) { QGraphicsView::mouseReleaseEvent(e); return; }
    QPointF sp = mapToScene(e->pos());

    // Measure tool release — create persistent measurement annotation
    if (m_measuring) {
        m_measuring = false;
        m_measureEnd = sp;
        double dx = m_measureEnd.x() - m_measureStart.x();
        double dy = m_measureEnd.y() - m_measureStart.y();
        double dist = std::sqrt(dx * dx + dy * dy);
        if (dist > 5) {
            auto *m = new MeasureAnnotation(m_measureStart, m_measureEnd);
            m_scene->addItem(m);
            record(m);
            m_activeItem = m;
        }
        // Reset overlay points so the live preview clears
        m_measureStart = m_measureEnd = QPointF();
        viewport()->update();
        return;
    }

    if (m_tool == ToolSelect) { QGraphicsView::mouseReleaseEvent(e); return; }
    if (m_cropping) {
        m_cropping = false;
        viewport()->update();
        QRectF cr = m_cropR.normalized().intersected(m_pxItem->boundingRect());
        if (cr.width() > 5 && cr.height() > 5) {
            // Save full pre-crop state
            m_preCropScreenshot = m_screenshot;
            m_preCropItems.clear();
            m_preCropUndo = m_undo;
            m_preCropDeleteHistory = m_deleteHistory;

            // Render the cropped region including all annotations
            QPixmap cropped(cr.size().toSize());
            cropped.fill(Qt::transparent);
            QPainter painter(&cropped);
            painter.setRenderHint(QPainter::Antialiasing);
            m_scene->render(&painter, cropped.rect(), cr);
            painter.end();

            // Save annotations for undo (detach from scene, don't delete)
            for (auto *it : m_scene->items()) {
                if (it != m_pxItem) {
                    m_scene->removeItem(it);
                    m_preCropItems.append(it);
                }
            }
            m_undo.clear();
            m_deleteHistory.clear();

            // Apply crop — the new screenshot includes the drawn items
            m_screenshot = cropped;
            m_pxItem->setPixmap(m_screenshot);
            m_numCounter = 1;
            QRectF sr = m_pxItem->boundingRect().adjusted(-20, -20, 20, 20);
            m_scene->setSceneRect(sr);
            resetTransform();
            m_zoom = 1.0;
            emit zoomChanged(m_zoom);
            setCurrentTool(ToolSelect);
            emit imageCropped(m_screenshot);
            QTimer::singleShot(50, this, [this]() { zoomToFit(); });
        }
        return;
    }
    if (m_drawing && m_tool == ToolPen) {
        m_drawing = false;
        if (m_livePen) { record(m_livePen); m_activeItem = m_livePen; m_livePen = nullptr; }
        return;
    }
    if (m_drawing) {
        m_drawing = false;
        m_b = sp;
        addAnnotation(m_a, m_b);
        viewport()->update();
        return;
    }
    QGraphicsView::mouseReleaseEvent(e);
}

void CanvasView::drawForeground(QPainter *p, const QRectF &)
{
    p->setRenderHint(QPainter::Antialiasing);
    double inv = 1.0 / qMax(m_zoom, 0.01);

    // Live shape preview
    if (m_drawing && m_tool != ToolPen) {
        QPen pen(m_color, m_width * inv);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        p->setPen(pen);
        p->setBrush(Qt::NoBrush);
        QRectF r = QRectF(m_a, m_b).normalized();
        switch (m_tool) {
        case ToolRect:    p->drawRect(r); break;
        case ToolEllipse: p->drawEllipse(r); break;
        case ToolArrow: {
            // Shaft
            p->drawLine(m_a, m_b);
            // Arrowhead wings — same math as ArrowAnnotation::paint
            double angle = std::atan2(m_b.y() - m_a.y(), m_b.x() - m_a.x());
            double headLen = (10.0 + m_width * 2.0) * inv;
            double spread = M_PI / 6.0;
            QPointF w1 = m_b - QPointF(std::cos(angle - spread) * headLen,
                                        std::sin(angle - spread) * headLen);
            QPointF w2 = m_b - QPointF(std::cos(angle + spread) * headLen,
                                        std::sin(angle + spread) * headLen);
            p->drawLine(m_b, w1);
            p->drawLine(m_b, w2);
            break;
        }
        case ToolLine:    p->drawLine(m_a, m_b); break;
        case ToolBlur:
            p->setPen(QPen(QColor(200, 200, 200, 128), inv, Qt::DashLine));
            p->drawRect(r); break;
        default: break;
        }
    }

    // Crop overlay
    if (m_cropping) {
        QRectF cr = m_cropR.normalized();
        QRectF all = m_pxItem->boundingRect();
        p->setPen(Qt::NoPen);
        p->setBrush(QColor(0, 0, 0, 100));
        p->drawRect(QRectF(all.left(), all.top(), all.width(), cr.top() - all.top()));
        p->drawRect(QRectF(all.left(), cr.bottom(), all.width(), all.bottom() - cr.bottom()));
        p->drawRect(QRectF(all.left(), cr.top(), cr.left() - all.left(), cr.height()));
        p->drawRect(QRectF(cr.right(), cr.top(), all.right() - cr.right(), cr.height()));
        p->setPen(QPen(QColor("#6C5CE7"), 2 * inv, Qt::DashLine));
        p->setBrush(Qt::NoBrush);
        p->drawRect(cr);
    }

    // Measurement live preview (only while dragging)
    if (m_measuring && m_tool == ToolMeasure) {
        double dx = m_measureEnd.x() - m_measureStart.x();
        double dy = m_measureEnd.y() - m_measureStart.y();
        double dist = std::sqrt(dx * dx + dy * dy);

        // Dashed line
        p->setPen(QPen(QColor("#6C5CE7"), 2 * inv, Qt::DashDotLine));
        p->setBrush(Qt::NoBrush);
        p->drawLine(m_measureStart, m_measureEnd);

        // Endpoints
        p->setPen(Qt::NoPen);
        p->setBrush(QColor("#6C5CE7"));
        p->drawEllipse(m_measureStart, 4 * inv, 4 * inv);
        p->drawEllipse(m_measureEnd, 4 * inv, 4 * inv);

        // Distance label at midpoint
        QPointF mid = (m_measureStart + m_measureEnd) / 2.0;
        QString label = QString("%1 px").arg(qRound(dist));
        QFont f; f.setPixelSize(qRound(12 * inv)); f.setBold(true);
        p->setFont(f);
        QFontMetrics fm(f);
        QRectF tr = fm.boundingRect(label);
        tr.adjust(-5 * inv, -2 * inv, 5 * inv, 2 * inv);
        tr.moveCenter(mid + QPointF(0, -12 * inv));
        p->setPen(Qt::NoPen);
        p->setBrush(QColor("#6C5CE7"));
        p->drawRoundedRect(tr, 3 * inv, 3 * inv);
        p->setPen(Qt::white);
        p->drawText(tr, Qt::AlignCenter, label);
    }

    // Magnifier - only when tool active and mouse is over viewport
    if (m_tool == ToolMagnifier && underMouse()) {
        QPointF sp = mapToScene(m_mousePos);
        const int sz = 130;
        const double mg = 8.0;
        double src = sz / (mg * qMax(m_zoom, 0.01));

        // Paint magnifier directly - no intermediate QPixmap allocation
        p->save();
        p->resetTransform();

        int mx = m_mousePos.x() + 25, my = m_mousePos.y() - sz - 15;
        if (mx + sz > viewport()->width()) mx = m_mousePos.x() - sz - 25;
        if (my < 0) my = m_mousePos.y() + 25;

        QRectF magRect(mx, my, sz, sz);

        // Clip to circle
        QPainterPath clipPath;
        clipPath.addEllipse(magRect);
        p->setClipPath(clipPath);

        // Draw zoomed region from scene
        QRectF srcR(sp.x() - src / 2, sp.y() - src / 2, src, src);
        QRectF vis = srcR.intersected(m_pxItem->boundingRect());
        if (!vis.isEmpty()) {
            // Map visible portion to magnifier rect
            double ox = (vis.x() - srcR.x()) / src * sz + mx;
            double oy = (vis.y() - srcR.y()) / src * sz + my;
            double ow = vis.width() / src * sz;
            double oh = vis.height() / src * sz;
            p->drawPixmap(QRectF(ox, oy, ow, oh), m_screenshot,
                          QRectF(vis.x(), vis.y(), vis.width(), vis.height()));
        }

        // Crosshair
        p->setClipping(false);
        p->setPen(QPen(Qt::white, 1));
        double cx = mx + sz / 2.0, cy = my + sz / 2.0;
        p->drawLine(QPointF(cx, my), QPointF(cx, my + sz));
        p->drawLine(QPointF(mx, cy), QPointF(mx + sz, cy));

        // Border
        p->setPen(QPen(QColor("#6C5CE7"), 3));
        p->setBrush(Qt::NoBrush);
        p->drawEllipse(magRect.adjusted(1, 1, -1, -1));

        // Hex label
        QColor col = colorAt(m_mousePos);
        if (col.isValid()) {
            QRectF labelR(mx, my + sz - 20, sz, 20);
            p->setPen(Qt::NoPen);
            p->setBrush(QColor(0, 0, 0, 180));
            p->drawRect(labelR);
            p->setPen(Qt::white);
            p->setFont(QFont("Consolas", 9, QFont::Bold));
            p->drawText(labelR, Qt::AlignCenter, col.name().toUpper());
        }

        p->restore();
    }
}

// ---- annotation creation ----
void CanvasView::addAnnotation(const QPointF &a, const QPointF &b)
{
    QRectF r = QRectF(a, b).normalized();
    bool tiny = r.width() < 4 && r.height() < 4;

    if (tiny && m_tool == ToolText) {
        auto *t = new TextAnnotation("");
        t->setPos(a);
        t->setColor(m_color);
        t->setFontSize(qMax(12, m_width * 4));
        m_scene->addItem(t);
        record(t);
        t->startEditing();
        m_activeItem = t;
        return;
    }
    if (tiny) return;

    QGraphicsItem *it = nullptr;
    switch (m_tool) {
    case ToolRect: {
        auto *x = new RectAnnotation(QRectF(QPointF(0, 0), r.size()));
        x->setPos(r.topLeft()); x->setColor(m_color); x->setStrokeWidth(m_width);
        it = x; break;
    }
    case ToolEllipse: {
        auto *x = new EllipseAnnotation(QRectF(QPointF(0, 0), r.size()));
        x->setPos(r.topLeft()); x->setColor(m_color); x->setStrokeWidth(m_width);
        it = x; break;
    }
    case ToolArrow: {
        auto *x = new ArrowAnnotation(QLineF(QPointF(0, 0), b - a));
        x->setPos(a); x->setColor(m_color); x->setStrokeWidth(m_width);
        it = x; break;
    }
    case ToolLine: {
        auto *x = new QGraphicsLineItem(QLineF(QPointF(0, 0), b - a));
        x->setPos(a);
        x->setPen(QPen(m_color, m_width, Qt::SolidLine, Qt::RoundCap));
        x->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
        x->setZValue(100);
        it = x; break;
    }
    case ToolText: {
        auto *x = new TextAnnotation("");
        x->setPos(r.topLeft()); x->setColor(m_color);
        x->setFontSize(qMax(12, m_width * 4));
        it = x; break;
    }
    case ToolBlur: {
        // Only store the region needed, not the full screenshot
        QRect region = r.toRect().intersected(m_screenshot.rect());
        auto *x = new BlurAnnotation(QRectF(QPointF(0, 0), r.size()));
        x->setPos(r.topLeft());
        if (!region.isEmpty())
            x->setSourcePixmap(m_screenshot.copy(region));
        it = x; break;
    }
    default: break;
    }

    if (it) {
        m_scene->addItem(it);
        record(it);
        m_activeItem = it; // track for real-time size/color updates
        auto *ta = dynamic_cast<TextAnnotation *>(it);
        if (ta) ta->startEditing();
    }
}

// ---- undo/redo ----
void CanvasView::record(QGraphicsItem *it)
{
    m_undo.push(it);
    while (!m_redo.isEmpty()) delete m_redo.pop();
}

void CanvasView::undo()
{
    // 1. Undo last annotation creation
    if (!m_undo.isEmpty()) {
        auto *it = m_undo.pop();
        m_scene->removeItem(it);
        m_redo.push(it);
        return;
    }

    // 2. Undo last delete — restore the batch
    if (!m_deleteHistory.isEmpty()) {
        QList<QGraphicsItem*> batch = m_deleteHistory.pop();
        for (auto *it : batch) {
            m_scene->addItem(it);
            m_undo.push(it);
        }
        return;
    }

    // 3. Undo crop — restore screenshot + all annotation items
    if (!m_preCropScreenshot.isNull()) {
        // Save current (post-crop) state for redo
        m_postCropScreenshot = m_screenshot;

        // Clear current scene (post-crop items only)
        while (!m_redo.isEmpty()) delete m_redo.pop();
        for (auto *it : m_scene->items())
            if (it != m_pxItem) { m_scene->removeItem(it); delete it; }

        // Restore pre-crop screenshot
        m_screenshot = m_preCropScreenshot;
        m_pxItem->setPixmap(m_screenshot);

        QRectF sr = m_pxItem->boundingRect().adjusted(-20, -20, 20, 20);
        m_scene->setSceneRect(sr);
        resetTransform();
        m_zoom = 1.0;
        emit zoomChanged(m_zoom);

        // Restore annotation items
        for (auto *it : m_preCropItems)
            m_scene->addItem(it);
        m_undo = m_preCropUndo;
        m_deleteHistory = m_preCropDeleteHistory;

        // Clear pre-crop state so we don't undo the same crop twice
        m_preCropScreenshot = QPixmap();
        m_preCropItems.clear();
        m_preCropUndo.clear();
        m_preCropDeleteHistory.clear();

        setCurrentTool(ToolSelect);
        emit imageCropped(m_screenshot);
        QTimer::singleShot(50, this, [this]() { zoomToFit(); });
    }
    // 4. Nothing to undo — do nothing (no crash)
}

void CanvasView::redo()
{
    // 1. Redo annotation
    if (!m_redo.isEmpty()) {
        auto *it = m_redo.pop();
        m_scene->addItem(it);
        m_undo.push(it);
        return;
    }

    // 2. Redo crop — re-apply the crop
    if (!m_postCropScreenshot.isNull()) {
        // Save current state as pre-crop again
        m_preCropScreenshot = m_screenshot;
        m_preCropItems.clear();
        m_preCropUndo = m_undo;
        m_preCropDeleteHistory = m_deleteHistory;

        for (auto *it : m_scene->items()) {
            if (it != m_pxItem) {
                m_scene->removeItem(it);
                m_preCropItems.append(it);
            }
        }
        m_undo.clear();
        m_deleteHistory.clear();

        // Restore post-crop screenshot
        m_screenshot = m_postCropScreenshot;
        m_postCropScreenshot = QPixmap(); // free
        m_pxItem->setPixmap(m_screenshot);

        QRectF sr = m_pxItem->boundingRect().adjusted(-20, -20, 20, 20);
        m_scene->setSceneRect(sr);
        resetTransform();
        m_zoom = 1.0;
        emit zoomChanged(m_zoom);

        setCurrentTool(ToolSelect);
        emit imageCropped(m_screenshot);
        QTimer::singleShot(50, this, [this]() { zoomToFit(); });
    }
}

void CanvasView::startCropMode() { setCurrentTool(ToolCrop); }

void CanvasView::deleteSelectedItems()
{
    QList<QGraphicsItem*> targets = m_scene->selectedItems();

    // If nothing selected, target ALL user annotations
    if (targets.isEmpty()) {
        for (auto *it : m_scene->items()) {
            if (it != m_pxItem)
                targets.append(it);
        }
    }

    if (targets.isEmpty()) return;

    // Remove from scene but keep alive for undo
    QList<QGraphicsItem*> batch;
    for (auto *it : targets) {
        if (it == m_pxItem) continue;
        m_scene->removeItem(it);
        m_undo.removeAll(it);
        m_redo.removeAll(it);
        if (it == m_activeItem) m_activeItem = nullptr;
        batch.append(it);
    }

    if (!batch.isEmpty())
        m_deleteHistory.push(batch);
}

void CanvasView::replaceScreenshot(const QPixmap &pm)
{
    while (!m_undo.isEmpty()) delete m_undo.pop();
    while (!m_redo.isEmpty()) delete m_redo.pop();
    while (!m_deleteHistory.isEmpty()) {
        for (auto *it : m_deleteHistory.pop()) delete it;
    }
    for (auto *it : m_scene->items())
        if (it != m_pxItem) { m_scene->removeItem(it); delete it; }

    m_screenshot = pm;
    m_pxItem->setPixmap(m_screenshot);
    m_numCounter = 1;
    QRectF sr = m_pxItem->boundingRect().adjusted(-20, -20, 20, 20);
    m_scene->setSceneRect(sr);
    resetTransform();
    m_zoom = 1.0;
    emit zoomChanged(m_zoom);
    setCurrentTool(ToolSelect);
    QTimer::singleShot(50, this, [this]() { zoomToFit(); });
}

QColor CanvasView::colorAt(const QPoint &vp) const
{
    // Efficient: grab a single 1x1 pixel instead of converting the whole image
    QPointF sp = mapToScene(vp);
    int x = qRound(sp.x()), y = qRound(sp.y());
    if (x >= 0 && x < m_screenshot.width() && y >= 0 && y < m_screenshot.height()) {
        QPixmap px = m_screenshot.copy(x, y, 1, 1);
        QImage img = px.toImage();
        return img.pixelColor(0, 0);
    }
    return QColor();
}

QPixmap CanvasView::renderToPixmap() const
{
    QRectF sr = m_pxItem->boundingRect();
    QPixmap res(sr.size().toSize());
    res.fill(Qt::transparent);
    QPainter p(&res);
    p.setRenderHint(QPainter::Antialiasing);
    m_scene->render(&p, res.rect(), sr);
    return res;
}

void CanvasView::zoomToFit()
{
    if (m_screenshot.isNull()) return;
    resetTransform();
    m_zoom = 1.0;

    int vw = viewport()->width() - 10;
    int vh = viewport()->height() - 10;
    if (vw < 50 || vh < 50) return;

    double sx = (double)vw / m_screenshot.width();
    double sy = (double)vh / m_screenshot.height();
    double fit = qMin(sx, sy);

    if (fit < 1.0) {
        scale(fit, fit);
        m_zoom = fit;
    }
    emit zoomChanged(m_zoom);
}
