#ifndef SCREENSHOTMANAGER_H
#define SCREENSHOTMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QRect>
#include <QTimer>
#include <QVector>
#include <QAbstractNativeEventFilter>

class SnipOverlay;

class ScreenshotManager : public QObject
{
    Q_OBJECT

public:
    explicit ScreenshotManager(QObject *parent = nullptr);
    ~ScreenshotManager();

    void captureFullScreen();
    void captureSnip();
    void captureScrolling();
    void stopScrollCapture();

signals:
    void screenshotTaken(const QPixmap &screenshot);

private slots:
    void onSnipCompleted(const QRect &region);
    void onSnipCancelled();
    void onScrollSnipCompleted(const QRect &region);
    void scrollCaptureStep();

private:
    void cleanupOverlay();
    void finishScrollCapture();
    QPixmap stitchImages(const QVector<QPixmap> &images) const;
    int findOverlap(const QImage &top, const QImage &bottom) const;

    QPixmap m_fullScreenCapture;
    SnipOverlay *m_snipOverlay;

    QRect m_scrollRegion;
    QVector<QPixmap> m_scrollCaptures;
    QTimer *m_scrollTimer;
    QTimer *m_escapeTimer; // polls Escape key during scroll capture
    int m_scrollSteps;
    int m_maxScrollSteps;
    bool m_scrollingActive;
};

#endif // SCREENSHOTMANAGER_H
