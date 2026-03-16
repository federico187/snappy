#include "preferencesdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTabWidget>
#include <QFormLayout>
#include <QScrollArea>
#include <QCoreApplication>
#include <QColorDialog>

struct SC { QString key, label, def; };
static const QVector<SC> SCDEFS = {
    {"SnipScreenshot","Snip Screenshot","Ctrl+Shift+1"},
    {"ScrollCapture","Scrolling Capture","Ctrl+Shift+2"},
    {"FullScreenshot","Full Screenshot","Ctrl+Shift+3"},
};

static const char *DST =
    "QDialog{background:#2D2D2D;color:#DDD;}"
    "QGroupBox{border:1px solid #555;border-radius:6px;margin-top:10px;padding:15px 10px 10px;color:#DDD;}"
    "QGroupBox::title{subcontrol-origin:margin;left:12px;padding:0 4px;color:#6C5CE7;font-weight:bold;}"
    "QLineEdit,QKeySequenceEdit{background:#1E1E1E;border:1px solid #555;border-radius:4px;padding:4px 8px;color:#DDD;}"
    "QLineEdit:focus,QKeySequenceEdit:focus{border-color:#6C5CE7;}"
    "QCheckBox{color:#DDD;spacing:8px;}"
    "QSpinBox{background:#1E1E1E;border:1px solid #555;border-radius:4px;padding:4px 8px;color:#DDD;min-width:60px;}"
    "QSpinBox:focus{border-color:#6C5CE7;}"
    "QPushButton{background:#6C5CE7;color:white;border:none;border-radius:4px;padding:6px 16px;font-weight:bold;}"
    "QPushButton:hover{background:#5A4BD1;}"
    "QPushButton#sec{background:#444;}QPushButton#sec:hover{background:#555;}"
    "QTabWidget::pane{border:1px solid #444;background:#2D2D2D;}"
    "QTabBar::tab{background:#1E1E1E;color:#AAA;padding:8px 16px;border:1px solid #444;border-bottom:none;border-radius:4px 4px 0 0;}"
    "QTabBar::tab:selected{background:#2D2D2D;color:#6C5CE7;}"
    "QLabel{color:#DDD;}QScrollArea{border:none;background:#2D2D2D;}QScrollArea>QWidget>QWidget{background:#2D2D2D;}";

PreferencesDialog::PreferencesDialog(QSettings *s, QWidget *p) : QDialog(p), m_s(s)
{
    setWindowTitle("Preferences"); setFixedSize(480,520); setStyleSheet(DST);
    m_defaultColor = QColor(m_s->value("Defaults/Color", "#E74C3C").toString());
    build(); load();
}

void PreferencesDialog::build()
{
    auto *ml = new QVBoxLayout(this);
    auto *tabs = new QTabWidget(this);

    // ===== General tab =====
    auto *gt = new QWidget;
    auto *gl = new QVBoxLayout(gt);
    auto *sg = new QGroupBox("Storage", gt);
    auto *sl = new QVBoxLayout(sg);
    auto *pl = new QHBoxLayout;
    pl->addWidget(new QLabel("Save folder:"));
    m_pathEdit = new QLineEdit; pl->addWidget(m_pathEdit,1);
    auto *br = new QPushButton("Browse"); br->setObjectName("sec");
    connect(br,&QPushButton::clicked,this,&PreferencesDialog::browseFolder);
    pl->addWidget(br); sl->addLayout(pl);
    m_autoSave = new QCheckBox("Auto-save to folder"); sl->addWidget(m_autoSave);
    gl->addWidget(sg);
    auto *bg = new QGroupBox("Behavior");
    auto *bgl = new QVBoxLayout(bg);
    m_tray = new QCheckBox("Start minimized to tray"); bgl->addWidget(m_tray);
    m_startWithWindows = new QCheckBox("Start with Windows"); bgl->addWidget(m_startWithWindows);
    gl->addWidget(bg);
    gl->addStretch();
    tabs->addTab(gt,"General");

    // ===== Defaults tab =====
    auto *dt = new QWidget;
    dt->setStyleSheet("background:#2D2D2D;");
    auto *dl = new QVBoxLayout(dt);

    auto *cg = new QGroupBox("Default Color");
    auto *cgl = new QHBoxLayout(cg);
    m_colorSwatch = new QToolButton;
    m_colorSwatch->setFixedSize(32, 32);
    m_colorSwatch->setCursor(Qt::PointingHandCursor);
    updateColorSwatch();
    connect(m_colorSwatch, &QToolButton::clicked, this, &PreferencesDialog::pickDefaultColor);
    cgl->addWidget(m_colorSwatch);
    cgl->addWidget(new QLabel("Click to change the default annotation color"));
    cgl->addStretch();
    dl->addWidget(cg);

    auto *wg = new QGroupBox("Default Sizes");
    auto *wfl = new QFormLayout(wg);
    wfl->setLabelAlignment(Qt::AlignRight);

    m_widthArrow = new QSpinBox; m_widthArrow->setRange(1, 20);
    wfl->addRow("Arrows:", m_widthArrow);

    m_widthLine = new QSpinBox; m_widthLine->setRange(1, 20);
    wfl->addRow("Lines:", m_widthLine);

    m_widthShape = new QSpinBox; m_widthShape->setRange(1, 20);
    wfl->addRow("Rectangles / Circles:", m_widthShape);

    m_widthText = new QSpinBox; m_widthText->setRange(1, 20);
    wfl->addRow("Text:", m_widthText);

    dl->addWidget(wg);
    dl->addStretch();
    tabs->addTab(dt, "Defaults");

    // ===== Shortcuts tab =====
    auto *st = new QWidget;
    st->setStyleSheet("background:#2D2D2D;");
    auto *sol = new QVBoxLayout(st);
    auto *sa = new QScrollArea(st); sa->setWidgetResizable(true);
    auto *si = new QWidget;
    si->setStyleSheet("background:#2D2D2D;");
    auto *fm = new QFormLayout(si); fm->setLabelAlignment(Qt::AlignRight);
    for (const auto &d : SCDEFS) {
        auto *e = new QKeySequenceEdit(si); e->setFixedWidth(150);
        m_scEdits[d.key] = e;
        fm->addRow(d.label + ":", e);
    }
    sa->setWidget(si); sol->addWidget(sa);
    auto *rb = new QPushButton("Reset Defaults"); rb->setObjectName("sec");
    connect(rb,&QPushButton::clicked,this,&PreferencesDialog::resetShortcuts);
    sol->addWidget(rb);
    tabs->addTab(st,"Shortcuts");

    ml->addWidget(tabs);
    auto *bl = new QHBoxLayout; bl->addStretch();
    auto *ab = new QPushButton("Apply");
    connect(ab,&QPushButton::clicked,this,&PreferencesDialog::apply);
    bl->addWidget(ab);
    auto *cb = new QPushButton("Close"); cb->setObjectName("sec");
    connect(cb,&QPushButton::clicked,this,&QDialog::close);
    bl->addWidget(cb);
    ml->addLayout(bl);
}

void PreferencesDialog::updateColorSwatch()
{
    m_colorSwatch->setStyleSheet(QString(
        "QToolButton{background:%1;border:2px solid #555;border-radius:6px;}"
        "QToolButton:hover{border-color:#999;}"
    ).arg(m_defaultColor.name()));
}

void PreferencesDialog::pickDefaultColor()
{
    QColor c = QColorDialog::getColor(m_defaultColor, this, "Default Color");
    if (c.isValid()) {
        m_defaultColor = c;
        updateColorSwatch();
    }
}

void PreferencesDialog::load()
{
    m_pathEdit->setText(m_s->value("General/SavePath",
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString());
    m_autoSave->setChecked(m_s->value("General/AutoSave",false).toBool());
    m_tray->setChecked(m_s->value("General/StartMinimized",true).toBool());
    m_startWithWindows->setChecked(m_s->value("General/StartWithWindows",false).toBool());
    for (const auto &d : SCDEFS)
        m_scEdits[d.key]->setKeySequence(QKeySequence(m_s->value("Shortcuts/"+d.key,d.def).toString()));

    // Defaults
    m_defaultColor = QColor(m_s->value("Defaults/Color", "#E74C3C").toString());
    updateColorSwatch();
    m_widthArrow->setValue(m_s->value("Defaults/WidthArrow", 3).toInt());
    m_widthLine->setValue(m_s->value("Defaults/WidthLine", 3).toInt());
    m_widthShape->setValue(m_s->value("Defaults/WidthShape", 3).toInt());
    m_widthText->setValue(m_s->value("Defaults/WidthText", 3).toInt());
}

void PreferencesDialog::browseFolder()
{
    QString d = QFileDialog::getExistingDirectory(this,"Select Folder",m_pathEdit->text());
    if (!d.isEmpty()) m_pathEdit->setText(d);
}

void PreferencesDialog::apply()
{
    m_s->setValue("General/SavePath", m_pathEdit->text());
    m_s->setValue("General/AutoSave", m_autoSave->isChecked());
    m_s->setValue("General/StartMinimized", m_tray->isChecked());
    m_s->setValue("General/StartWithWindows", m_startWithWindows->isChecked());

#ifdef Q_OS_WIN
    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                  QSettings::NativeFormat);
    if (m_startWithWindows->isChecked()) {
        reg.setValue("Snappy", "\"" + QCoreApplication::applicationFilePath().replace("/","\\") + "\"");
    } else {
        reg.remove("Snappy");
    }
#endif

    for (const auto &d : SCDEFS)
        m_s->setValue("Shortcuts/"+d.key, m_scEdits[d.key]->keySequence().toString());

    // Defaults
    m_s->setValue("Defaults/Color", m_defaultColor.name());
    m_s->setValue("Defaults/WidthArrow", m_widthArrow->value());
    m_s->setValue("Defaults/WidthLine", m_widthLine->value());
    m_s->setValue("Defaults/WidthShape", m_widthShape->value());
    m_s->setValue("Defaults/WidthText", m_widthText->value());

    m_s->sync(); close();
}

void PreferencesDialog::resetShortcuts()
{
    for (const auto &d : SCDEFS) m_scEdits[d.key]->setKeySequence(QKeySequence(d.def));
}
