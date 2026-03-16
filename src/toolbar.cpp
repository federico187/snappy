#include "toolbar.h"
#include <QFrame>
#include <QPainter>

static const char *TB_STYLE =
    "QToolButton{background:transparent;border:none;border-radius:5px;"
    "padding:4px;min-width:26px;min-height:26px;}"
    "QToolButton:hover{background:rgba(255,255,255,0.08);}"
    "QToolButton:checked{background:rgba(255,255,255,0.15);}";

static const char *ACT_STYLE =
    "QToolButton{background:transparent;border:none;border-radius:5px;"
    "padding:4px;min-width:26px;min-height:26px;}"
    "QToolButton:hover{background:rgba(255,255,255,0.08);}"
    "QToolButton:pressed{background:rgba(255,255,255,0.12);}";

QIcon Toolbar::paintIcon(const QString &id)
{
    QPixmap pm(20, 20);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen(QColor("#CCC"), 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);

    if (id == "cursor") {
        QPolygonF a;
        a << QPointF(4,2) << QPointF(4,16) << QPointF(8,12)
          << QPointF(13,16) << QPointF(15,14) << QPointF(10,10) << QPointF(15,8);
        p.setBrush(QColor("#CCC")); p.setPen(Qt::NoPen); p.drawPolygon(a);
    } else if (id == "crop") {
        p.drawLine(6,2,6,14); p.drawLine(2,6,14,6);
        p.drawLine(14,6,14,18); p.drawLine(6,14,18,14);
    } else if (id == "copy") {
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(3,5,10,12,2,2); p.drawRoundedRect(7,2,10,12,2,2);
    } else if (id == "save") {
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(3,2,14,16,2,2); p.drawRect(6,2,8,5); p.drawRect(6,12,8,4);
    } else if (id == "undo") {
        p.setBrush(Qt::NoBrush);
        QPainterPath arc; arc.moveTo(6,8); arc.arcTo(4,4,12,12,130,-200); p.drawPath(arc);
        p.drawLine(6,8,6,4); p.drawLine(6,8,10,8);
    } else if (id == "redo") {
        p.setBrush(Qt::NoBrush);
        QPainterPath arc; arc.moveTo(14,8); arc.arcTo(4,4,12,12,50,200); p.drawPath(arc);
        p.drawLine(14,8,14,4); p.drawLine(14,8,10,8);
    } else if (id == "arrow") {
        // Simple unfilled arrow: shaft + two wing lines
        p.setPen(QPen(QColor("#CCC"), 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawLine(3, 16, 17, 4);   // shaft
        p.drawLine(17, 4, 11, 5);   // wing 1
        p.drawLine(17, 4, 16, 10);  // wing 2
    } else if (id == "text") {
        QFont f = p.font(); f.setPixelSize(15); f.setBold(true); p.setFont(f);
        p.drawText(pm.rect(), Qt::AlignCenter, "T");
    } else if (id == "num") {
        p.setBrush(Qt::NoBrush); p.drawEllipse(3,3,14,14);
        QFont f = p.font(); f.setPixelSize(11); f.setBold(true); p.setFont(f);
        p.drawText(pm.rect(), Qt::AlignCenter, "1");
    } else if (id == "rect") {
        p.setBrush(Qt::NoBrush); p.drawRect(3,4,14,12);
    } else if (id == "ellipse") {
        p.setBrush(Qt::NoBrush); p.drawEllipse(3,4,14,12);
    } else if (id == "line") {
        p.drawLine(4,16,16,4);
    } else if (id == "pen") {
        QPainterPath c; c.moveTo(4,14); c.cubicTo(7,6,12,12,16,4);
        p.setBrush(Qt::NoBrush); p.drawPath(c);
    } else if (id == "ruler") {
        // Horizontal ruler body
        p.setPen(QPen(QColor("#CCC"), 1.2));
        p.setBrush(Qt::NoBrush);
        p.drawRect(2, 6, 16, 8); // ruler body
        // Tick marks along top edge (varying heights)
        p.drawLine(5, 6, 5, 10);   // long tick
        p.drawLine(8, 6, 8, 9);    // short tick
        p.drawLine(11, 6, 11, 10); // long tick
        p.drawLine(14, 6, 14, 9);  // short tick
        // Small numbers
        QFont f = p.font(); f.setPixelSize(6); p.setFont(f);
        p.drawText(QRect(3, 10, 6, 5), Qt::AlignCenter, "0");
        p.drawText(QRect(12, 10, 6, 5), Qt::AlignCenter, "5");
    } else if (id == "blur") {
        p.setPen(QPen(QColor("#999"), 1));
        for (int y = 4; y < 17; y += 3)
            for (int x = 3; x < 18; x += 3) p.drawPoint(x, y);
    } else if (id == "mag") {
        p.setBrush(Qt::NoBrush); p.drawEllipse(3,3,10,10);
        p.setPen(QPen(QColor("#CCC"), 2.2, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(12,12,17,17);
    } else if (id == "del") {
        p.drawLine(5,5,15,5); p.drawLine(8,3,12,3);
        p.drawLine(6,5,6,16); p.drawLine(14,5,14,16); p.drawLine(6,16,14,16);
        p.drawLine(9,7,9,14); p.drawLine(11,7,11,14);
    }
    p.end();
    return QIcon(pm);
}

QWidget *Toolbar::addSep(QHBoxLayout *lay)
{
    auto *f = new QFrame(this);
    f->setFixedSize(1, 22);
    f->setStyleSheet("background:rgba(255,255,255,0.1);");
    lay->addWidget(f); return f;
}

QToolButton *Toolbar::addTool(QHBoxLayout *lay, const QString &iconId,
                               const QString &tip, CanvasView::Tool tool, bool hasWidth)
{
    auto *b = new QToolButton(this);
    b->setIcon(paintIcon(iconId)); b->setIconSize(QSize(20, 20));
    b->setToolTip(tip); b->setCheckable(true); b->setStyleSheet(TB_STYLE);
    m_grp->addButton(b); m_map[tool] = b;
    if (hasWidth) m_widthTools.insert(static_cast<int>(tool));
    connect(b, &QToolButton::clicked, [this, tool]() {
        emit toolChanged(tool);
        updateWidthBarVisibility(tool);
    });
    lay->addWidget(b); return b;
}

QToolButton *Toolbar::addAction(QHBoxLayout *lay, const QString &iconId, const QString &tip)
{
    auto *b = new QToolButton(this);
    b->setIcon(paintIcon(iconId)); b->setIconSize(QSize(20, 20));
    b->setToolTip(tip); b->setCheckable(false); b->setStyleSheet(ACT_STYLE);
    lay->addWidget(b); return b;
}

void Toolbar::updateWidthBarVisibility(CanvasView::Tool tool)
{
    if (m_widthBar)
        m_widthBar->setVisible(m_widthTools.contains(static_cast<int>(tool)));
}

Toolbar::Toolbar(QWidget *parent)
    : QWidget(parent), m_grp(new QButtonGroup(this)), m_widthBar(nullptr)
{
    m_grp->setExclusive(true);
    setFixedHeight(38);
    setStyleSheet("background:#2B2B2F;");

    auto *L = new QHBoxLayout(this);
    L->setContentsMargins(6, 3, 6, 3);
    L->setSpacing(2);

    auto *sel = addTool(L, "cursor", "Select (V)", CanvasView::ToolSelect);
    sel->setChecked(true);
    auto *crop = addAction(L, "crop", "Crop");
    connect(crop, &QToolButton::clicked, this, &Toolbar::cropRequested);
    addSep(L);
    auto *cp = addAction(L, "copy", "Copy (Ctrl+C)");
    connect(cp, &QToolButton::clicked, this, &Toolbar::copyRequested);
    auto *sv = addAction(L, "save", "Save (Ctrl+S)");
    connect(sv, &QToolButton::clicked, this, &Toolbar::saveRequested);
    addSep(L);
    auto *un = addAction(L, "undo", "Undo (Ctrl+Z)");
    connect(un, &QToolButton::clicked, this, &Toolbar::undoRequested);
    auto *re = addAction(L, "redo", "Redo (Ctrl+Y)");
    connect(re, &QToolButton::clicked, this, &Toolbar::redoRequested);
    addSep(L);
    addTool(L, "arrow",     "Arrow (A)",     CanvasView::ToolArrow,   true);
    addTool(L, "text",      "Text (T)",      CanvasView::ToolText,    true);
    addTool(L, "num",       "Numbered (N)",  CanvasView::ToolNumbered, true);
    addSep(L);
    addTool(L, "rect",      "Rectangle (R)", CanvasView::ToolRect,    true);
    addTool(L, "ellipse",   "Ellipse (E)",   CanvasView::ToolEllipse, true);
    addTool(L, "line",      "Line (L)",      CanvasView::ToolLine,    true);
    addSep(L);
    addTool(L, "pen",       "Pen (P)",       CanvasView::ToolPen,     true);
    addTool(L, "ruler",     "Measure (H)",   CanvasView::ToolMeasure);
    addTool(L, "blur",      "Blur (B)",      CanvasView::ToolBlur);
    addTool(L, "mag",       "Magnifier",     CanvasView::ToolMagnifier);
    addSep(L);
    auto *dl = addAction(L, "del", "Delete (Del)");
    connect(dl, &QToolButton::clicked, this, &Toolbar::deleteRequested);
    L->addStretch();

    // ===== Horizontal width bar (separate widget row) =====
    m_widthBar = new QWidget();
    m_widthBar->setFixedHeight(26);
    m_widthBar->setStyleSheet("background:#252528;border-bottom:1px solid #1A1A1D;");
    auto *wl = new QHBoxLayout(m_widthBar);
    wl->setContentsMargins(12, 2, 12, 2);
    wl->setSpacing(8);

    auto *wLbl = new QLabel("Size:", m_widthBar);
    wLbl->setStyleSheet("color:#888;font-size:11px;background:transparent;");
    wl->addWidget(wLbl);

    m_widthSlider = new QSlider(Qt::Horizontal, m_widthBar);
    m_widthSlider->setRange(1, 20);
    m_widthSlider->setValue(3);
    m_widthSlider->setFixedWidth(180);
    m_widthSlider->setStyleSheet(
        "QSlider::groove:horizontal{background:#444;height:4px;border-radius:2px;}"
        "QSlider::handle:horizontal{background:#6C5CE7;width:12px;height:12px;"
        "margin:-4px 0;border-radius:6px;}");
    wl->addWidget(m_widthSlider);

    m_widthLabel = new QLabel("3", m_widthBar);
    m_widthLabel->setFixedWidth(22);
    m_widthLabel->setAlignment(Qt::AlignCenter);
    m_widthLabel->setStyleSheet("color:#DDD;font-size:11px;font-weight:bold;background:transparent;");
    wl->addWidget(m_widthLabel);
    wl->addStretch();

    connect(m_widthSlider, &QSlider::valueChanged, [this](int v) {
        m_widthLabel->setText(QString::number(v));
        emit strokeWidthChanged(v);
    });

    m_widthBar->setVisible(false);
}

void Toolbar::setActiveTool(CanvasView::Tool t)
{
    if (m_map.contains(t)) m_map[t]->setChecked(true);
    updateWidthBarVisibility(t);
}

void Toolbar::setSliderValue(int v)
{
    if (m_widthSlider) {
        m_widthSlider->blockSignals(true);
        m_widthSlider->setValue(v);
        m_widthSlider->blockSignals(false);
        if (m_widthLabel) m_widthLabel->setText(QString::number(v));
    }
}
