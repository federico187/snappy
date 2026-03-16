#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QSettings>
#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include "canvasview.h"

class Toolbar;

class EditorWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit EditorWindow(const QPixmap &screenshot, QSettings *settings,
                          QWidget *parent = nullptr);
private slots:
    void onColorPicked(const QColor &c);
    void onPixelInfo(const QPoint &pos, const QColor &c);
    void onZoom(qreal z);
    void onCropped(const QPixmap &img);
    void save();
    void copy();
    void pickColor();
    void pickFromImage();
private:
    void buildUI();
    void setupShortcuts();
    void fitWindow();
    void setSwatchColor(const QColor &c);
    void activateTool(CanvasView::Tool t);
    void applyDefaults();
    void onToolChanged(CanvasView::Tool t);

    QPixmap m_shot;
    QSettings *m_set;
    CanvasView *m_canvas;
    Toolbar *m_bar;
    QToolButton *m_swatch;
    QPushButton *m_hexLabel;
    QLabel *m_hintLabel;
    QLabel *m_sizeLabel;
    QLabel *m_zoomLabel;

    // Per-tool default widths
    int m_defWidthArrow;
    int m_defWidthLine;
    int m_defWidthShape;
    int m_defWidthText;
};

#endif
