#include "editorwindow.h"
#include "canvasview.h"
#include "toolbar.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QKeyEvent>
#include <QMessageBox>
#include <QLabel>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <QShortcut>
#include <QTimer>
#include <QFrame>
#include <QColorDialog>
#include <QScreen>
#include <QAction>
#include <QGraphicsTextItem>
#include <QGraphicsScene>

EditorWindow::EditorWindow(const QPixmap &screenshot, QSettings *settings, QWidget *parent)
    : QMainWindow(parent)
    , m_shot(screenshot)
    , m_set(settings)
    , m_canvas(nullptr)
    , m_bar(nullptr)
    , m_swatch(nullptr)
    , m_hexLabel(nullptr)
    , m_hintLabel(nullptr)
    , m_sizeLabel(nullptr)
    , m_zoomLabel(nullptr)
{
    setWindowTitle("Snappy - Screenshot Tool");
    setMinimumSize(400, 300);
    setStyleSheet("QMainWindow{background:#1E1E22;}");
    buildUI();
    setupShortcuts();
    applyDefaults();
    fitWindow();
    // zoomToFit after layout is settled
    QTimer::singleShot(0, this, [this](){ if(m_canvas) m_canvas->zoomToFit(); });
}

void EditorWindow::applyDefaults()
{
    // Read per-tool defaults from settings
    m_defWidthArrow = m_set->value("Defaults/WidthArrow", 3).toInt();
    m_defWidthLine  = m_set->value("Defaults/WidthLine", 3).toInt();
    m_defWidthShape = m_set->value("Defaults/WidthShape", 3).toInt();
    m_defWidthText  = m_set->value("Defaults/WidthText", 3).toInt();

    // Apply default color
    QColor defColor(m_set->value("Defaults/Color", "#E74C3C").toString());
    if (defColor.isValid() && m_canvas) {
        m_canvas->setDrawingColor(defColor);
        setSwatchColor(defColor);
    }

    // Apply initial width (arrow default as starting point)
    if (m_canvas) m_canvas->setStrokeWidth(m_defWidthArrow);
    if (m_bar) m_bar->setSliderValue(m_defWidthArrow);
}

void EditorWindow::onToolChanged(CanvasView::Tool t)
{
    // Set per-tool default width on the slider
    int w = m_defWidthArrow; // fallback
    switch (t) {
    case CanvasView::ToolArrow:
        w = m_defWidthArrow; break;
    case CanvasView::ToolLine:
    case CanvasView::ToolPen:
        w = m_defWidthLine; break;
    case CanvasView::ToolRect:
    case CanvasView::ToolEllipse:
        w = m_defWidthShape; break;
    case CanvasView::ToolText:
        w = m_defWidthText; break;
    case CanvasView::ToolNumbered:
        w = m_defWidthShape; break;
    default: break;
    }

    if (m_bar) m_bar->setSliderValue(w);
    if (m_canvas) {
        m_canvas->setStrokeWidth(w);
        m_canvas->setCurrentTool(t);
    }
}

void EditorWindow::fitWindow()
{
    QScreen *scr = QApplication::primaryScreen();
    if (!scr) { resize(900, 600); return; }
    QRect av = scr->availableGeometry();
    int minW = 850; // wide enough for all toolbar icons
    int w = qBound(minW, m_shot.width() + 20, av.width() * 85 / 100);
    int h = qBound(400, m_shot.height() + 62, av.height() * 85 / 100);
    resize(w, h);
    move(av.center() - QPoint(width()/2, height()/2));
}

void EditorWindow::buildUI()
{
    QWidget *c = new QWidget(this);
    setCentralWidget(c);
    auto *vlay = new QVBoxLayout(c);
    vlay->setContentsMargins(0,0,0,0);
    vlay->setSpacing(0);

    // ===== TOP BAR =====
    QWidget *top = new QWidget(c);
    top->setFixedHeight(42);
    top->setStyleSheet("background:#2B2B2F;border-bottom:1px solid #1A1A1D;");
    auto *tlay = new QHBoxLayout(top);
    tlay->setContentsMargins(0,0,10,0);
    tlay->setSpacing(0);

    m_bar = new Toolbar(top);
    tlay->addWidget(m_bar, 1);

    // ---- right info section ----
    auto *info = new QWidget(top);
    info->setStyleSheet("background:transparent;");
    auto *il = new QHBoxLayout(info);
    il->setContentsMargins(0,0,0,0);
    il->setSpacing(6);

    auto mkSep = [info](){
        auto *f = new QFrame(info); f->setFixedSize(1,26);
        f->setStyleSheet("background:rgba(255,255,255,0.1);"); return f;
    };

    // Color swatch
    m_swatch = new QToolButton(info);
    m_swatch->setFixedSize(24,24);
    m_swatch->setToolTip("Click to choose color");
    setSwatchColor(QColor("#E74C3C"));
    connect(m_swatch, &QToolButton::clicked, this, &EditorWindow::pickColor);
    il->addWidget(m_swatch);

    // Pick from image button
    auto *pfb = new QToolButton(info);
    pfb->setText("💧"); // 💧 waterdrop
    pfb->setFixedSize(22, 24);
    pfb->setToolTip("Pick color from image");
    pfb->setStyleSheet(
        "QToolButton{background:#444;border:none;font-size:14px;"
        "color:#DDD;border-radius:4px;padding:0;}"
        "QToolButton:hover{background:rgba(255,255,255,0.15);}");
    connect(pfb, &QToolButton::clicked, this, &EditorWindow::pickFromImage);
    il->addWidget(pfb);

    // Hex + hint
    auto *cw = new QWidget(info);
    auto *cl = new QVBoxLayout(cw);
    cl->setContentsMargins(0,0,0,0); cl->setSpacing(0);
    m_hexLabel = new QPushButton("#E74C3C", cw);
    m_hexLabel->setFlat(true);
    m_hexLabel->setCursor(QCursor(Qt::PointingHandCursor));
    m_hexLabel->setStyleSheet(
        "QPushButton{color:#DDD;font-family:Consolas,monospace;font-size:12px;"
        "font-weight:bold;border:none;background:transparent;text-align:left;padding:0;}"
        "QPushButton:hover{color:#FFF;text-decoration:underline;}");
    connect(m_hexLabel, &QPushButton::clicked, [this]() {
        QApplication::clipboard()->setText(m_hexLabel->text());
        if (m_hintLabel) {
            m_hintLabel->setText("Copied!");
            QTimer::singleShot(1500, this, [this]() {
                if (m_hintLabel) m_hintLabel->setText("Click to copy");
            });
        }
    });
    m_hintLabel = new QLabel("Click to copy", cw);
    m_hintLabel->setStyleSheet("color:#666;font-size:10px;");
    cl->addWidget(m_hexLabel);
    cl->addWidget(m_hintLabel);
    il->addWidget(cw);

    il->addWidget(mkSep());

    // Size
    auto *sw = new QWidget(info);
    auto *sl2 = new QVBoxLayout(sw);
    sl2->setContentsMargins(0,0,0,0); sl2->setSpacing(0);
    m_sizeLabel = new QLabel(QString("%1\u00D7%2").arg(m_shot.width()).arg(m_shot.height()), sw);
    m_sizeLabel->setStyleSheet("color:#DDD;font-size:12px;font-weight:bold;");
    auto *sh = new QLabel("Image size", sw);
    sh->setStyleSheet("color:#666;font-size:10px;");
    sl2->addWidget(m_sizeLabel);
    sl2->addWidget(sh);
    il->addWidget(sw);

    il->addWidget(mkSep());

    // Zoom
    auto *zw = new QWidget(info);
    auto *zl2 = new QVBoxLayout(zw);
    zl2->setContentsMargins(0,0,0,0); zl2->setSpacing(0);
    m_zoomLabel = new QLabel("100%", zw);
    m_zoomLabel->setStyleSheet("color:#DDD;font-size:12px;font-weight:bold;");
    auto *zh = new QLabel("Zoom", zw);
    zh->setStyleSheet("color:#666;font-size:10px;");
    zl2->addWidget(m_zoomLabel);
    zl2->addWidget(zh);
    il->addWidget(zw);

    tlay->addWidget(info);
    vlay->addWidget(top);

    // ===== WIDTH BAR (horizontal, below toolbar, shows/hides per tool) =====
    vlay->addWidget(m_bar->widthBar());

    // ===== CANVAS =====
    m_canvas = new CanvasView(m_shot, c);
    vlay->addWidget(m_canvas, 1);

    // ===== CONNECTIONS =====
    connect(m_bar, &Toolbar::toolChanged, this, &EditorWindow::onToolChanged);
    connect(m_bar, &Toolbar::deleteRequested, m_canvas, &CanvasView::deleteSelectedItems);
    connect(m_bar, &Toolbar::copyRequested, this, &EditorWindow::copy);
    connect(m_bar, &Toolbar::saveRequested, this, &EditorWindow::save);
    connect(m_bar, &Toolbar::undoRequested, m_canvas, &CanvasView::undo);
    connect(m_bar, &Toolbar::redoRequested, m_canvas, &CanvasView::redo);
    connect(m_bar, &Toolbar::cropRequested, m_canvas, &CanvasView::startCropMode);
    connect(m_bar, &Toolbar::strokeWidthChanged, m_canvas, &CanvasView::setStrokeWidth);

    connect(m_canvas, &CanvasView::colorPicked, this, &EditorWindow::onColorPicked);
    connect(m_canvas, &CanvasView::pixelInfoChanged, this, &EditorWindow::onPixelInfo);
    connect(m_canvas, &CanvasView::zoomChanged, this, &EditorWindow::onZoom);
    connect(m_canvas, &CanvasView::imageCropped, this, &EditorWindow::onCropped);
    connect(m_canvas, &CanvasView::toolSwitchedToSelect, [this]() {
        m_bar->setActiveTool(CanvasView::ToolSelect);
    });

    // Tab = copy hex
    auto *tabShort = new QShortcut(Qt::Key_Tab, this);
    connect(tabShort, &QShortcut::activated, [this](){
        if (!m_hexLabel) return;
        QApplication::clipboard()->setText(m_hexLabel->text());
        m_hintLabel->setText("Copied!");
        QTimer::singleShot(1500, this, [this](){ if(m_hintLabel) m_hintLabel->setText("Click to copy"); });
    });
}

// ---- slots ----
void EditorWindow::onColorPicked(const QColor &c)
{
    // Only set drawing color for future annotations — don't change existing items
    m_canvas->setDrawingColor(c);
    setSwatchColor(c);
    if (m_canvas->currentTool() == CanvasView::ToolColorPick) {
        m_canvas->setCurrentTool(CanvasView::ToolSelect);
        m_bar->setActiveTool(CanvasView::ToolSelect);
    }
}

void EditorWindow::onPixelInfo(const QPoint &, const QColor &c)
{
    if (c.isValid() && m_hexLabel)
        m_hexLabel->setText(c.name().toUpper());
}

void EditorWindow::onZoom(qreal z)
{
    if (m_zoomLabel)
        m_zoomLabel->setText(QString("%1%").arg(qRound(z * 100)));
}

void EditorWindow::onCropped(const QPixmap &img)
{
    // Canvas already updated internally — just update the size label
    m_shot = img;
    if (m_sizeLabel)
        m_sizeLabel->setText(QString("%1\u00D7%2").arg(img.width()).arg(img.height()));
    m_bar->setActiveTool(CanvasView::ToolSelect);
    fitWindow();
}

void EditorWindow::pickColor()
{
    QColor c = QColorDialog::getColor(
        m_canvas ? m_canvas->currentColor() : QColor("#E74C3C"), this, "Choose Color");
    if (c.isValid() && m_canvas) {
        m_canvas->applyColorToSelection(c);
        setSwatchColor(c);
    }
}

void EditorWindow::pickFromImage()
{
    if (m_canvas) {
        m_canvas->setCurrentTool(CanvasView::ToolColorPick);
        m_canvas->setFocus(); // ensure canvas receives the next click
        setCursor(Qt::CrossCursor);
    }
}

void EditorWindow::setSwatchColor(const QColor &c)
{
    if (m_hexLabel)  m_hexLabel->setText(c.name().toUpper());
    if (m_swatch)
        m_swatch->setStyleSheet(QString(
            "QToolButton{background:%1;border:2px solid #555;border-radius:12px;}"
            "QToolButton:hover{border-color:#999;}"
        ).arg(c.name()));
}

void EditorWindow::save()
{
    QString dp = m_set->value("General/SavePath",
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
    QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    QString df = dp + "/snappy_" + ts + ".png";
    bool autoS = m_set->value("General/AutoSave", false).toBool();
    QString fp;
    if (autoS) { QDir().mkpath(dp); fp = df; }
    else fp = QFileDialog::getSaveFileName(this, "Save", df,
             "PNG (*.png);;JPEG (*.jpg);;BMP (*.bmp);;All Files (*)");
    if (fp.isEmpty() || !m_canvas) return;
    if (m_canvas->renderToPixmap().save(fp)) {
        if (m_hintLabel) { m_hintLabel->setText("Saved!");
            QTimer::singleShot(2000, this, [this](){ if(m_hintLabel) m_hintLabel->setText("Click to copy"); }); }
    } else QMessageBox::warning(this, "Error", "Failed to save.");
}

void EditorWindow::copy()
{
    if (!m_canvas) return;
    QApplication::clipboard()->setPixmap(m_canvas->renderToPixmap());
    hide();
}

void EditorWindow::activateTool(CanvasView::Tool t)
{
    // Don't switch tools if user is currently editing text
    if (m_canvas && m_canvas->scene()) {
        auto *fi = m_canvas->scene()->focusItem();
        if (fi) {
            auto *ti = dynamic_cast<QGraphicsTextItem*>(fi);
            if (ti && (ti->textInteractionFlags() & Qt::TextEditorInteraction))
                return; // user is typing, ignore shortcut
        }
    }
    if (m_canvas) m_canvas->setCurrentTool(t);
    if (m_bar) m_bar->setActiveTool(t);
}

void EditorWindow::setupShortcuts()
{
    // Helper: create a shortcut from settings with a default
    auto sc = [this](const QString &name, const QString &def) -> QKeySequence {
        return QKeySequence(m_set->value("Shortcuts/" + name, def).toString());
    };

    // Tool shortcuts
    auto toolSC = [this, &sc](const QString &n, const QString &d, CanvasView::Tool t) {
        auto *s = new QShortcut(sc(n, d), this);
        s->setContext(Qt::WindowShortcut);
        connect(s, &QShortcut::activated, [this, t]() { activateTool(t); });
    };

    toolSC("Select",    "V", CanvasView::ToolSelect);
    toolSC("Arrow",     "A", CanvasView::ToolArrow);
    toolSC("Text",      "T", CanvasView::ToolText);
    toolSC("Numbered",  "N", CanvasView::ToolNumbered);
    toolSC("Rectangle", "R", CanvasView::ToolRect);
    toolSC("Ellipse",   "E", CanvasView::ToolEllipse);
    toolSC("Line",      "L", CanvasView::ToolLine);
    toolSC("Pen",       "P", CanvasView::ToolPen);
    toolSC("Measure", "H", CanvasView::ToolMeasure);
    toolSC("Blur",      "B", CanvasView::ToolBlur);

    // Action shortcuts
    auto *sSave = new QShortcut(sc("Save","Ctrl+S"), this);
    sSave->setContext(Qt::WindowShortcut);
    connect(sSave, &QShortcut::activated, this, &EditorWindow::save);

    auto *sCopy = new QShortcut(sc("Copy","Ctrl+C"), this);
    sCopy->setContext(Qt::WindowShortcut);
    connect(sCopy, &QShortcut::activated, this, &EditorWindow::copy);

    auto *sUndo = new QShortcut(sc("Undo","Ctrl+Z"), this);
    sUndo->setContext(Qt::WindowShortcut);
    connect(sUndo, &QShortcut::activated, [this](){ if(m_canvas) m_canvas->undo(); });

    auto *sRedo = new QShortcut(sc("Redo","Ctrl+Y"), this);
    sRedo->setContext(Qt::WindowShortcut);
    connect(sRedo, &QShortcut::activated, [this](){ if(m_canvas) m_canvas->redo(); });

    auto *sCrop = new QShortcut(sc("Crop","Ctrl+Shift+X"), this);
    sCrop->setContext(Qt::WindowShortcut);
    connect(sCrop, &QShortcut::activated, [this](){ if(m_canvas) m_canvas->startCropMode(); });

    // Delete
    auto *sDel = new QShortcut(QKeySequence::Delete, this);
    sDel->setContext(Qt::WindowShortcut);
    connect(sDel, &QShortcut::activated, [this](){ if(m_canvas) m_canvas->deleteSelectedItems(); });

    auto *sDel2 = new QShortcut(QKeySequence(Qt::Key_Backspace), this);
    sDel2->setContext(Qt::WindowShortcut);
    connect(sDel2, &QShortcut::activated, [this](){ if(m_canvas) m_canvas->deleteSelectedItems(); });

    // Escape
    auto *sEsc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    sEsc->setContext(Qt::WindowShortcut);
    connect(sEsc, &QShortcut::activated, this, &QWidget::close);
}
