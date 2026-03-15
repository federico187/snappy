#ifndef SNIPOVERLAY_H
#define SNIPOVERLAY_H

#include <QWidget>
#include <QPixmap>
#include <QPoint>
#include <QRect>

class SnipOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit SnipOverlay(const QPixmap &bg, QWidget *parent = nullptr);
signals:
    void regionSelected(const QRect &r);
    void cancelled();
protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
private:
    QPixmap m_bg;
    QPoint m_p1, m_p2;
    bool m_sel;
};

#endif
