#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QWidget>
#include <QToolButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QMap>
#include <QSet>
#include "canvasview.h"

class Toolbar : public QWidget
{
    Q_OBJECT
public:
    explicit Toolbar(QWidget *parent = nullptr);
    void setActiveTool(CanvasView::Tool tool);
    QWidget *widthBar() { return m_widthBar; } // for editor to add to layout
signals:
    void toolChanged(CanvasView::Tool tool);
    void undoRequested();
    void redoRequested();
    void cropRequested();
    void copyRequested();
    void saveRequested();
    void deleteRequested();
    void strokeWidthChanged(int w);
private:
    QToolButton *addTool(QHBoxLayout *lay, const QString &iconId,
                         const QString &tip, CanvasView::Tool tool, bool hasWidth = false);
    QToolButton *addAction(QHBoxLayout *lay, const QString &iconId, const QString &tip);
    QWidget *addSep(QHBoxLayout *lay);
    static QIcon paintIcon(const QString &id);
    void updateWidthBarVisibility(CanvasView::Tool tool);

    QButtonGroup *m_grp;
    QMap<CanvasView::Tool, QToolButton*> m_map;
    QWidget *m_widthBar;
    QSlider *m_widthSlider;
    QLabel *m_widthLabel;
    QSet<int> m_widthTools; // tools that show the width bar
};

#endif
