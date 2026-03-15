#include "screenshotmanager.h"
#include "snipoverlay.h"
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QImage>
#include <QPainter>
#include <QCursor>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

ScreenshotManager::ScreenshotManager(QObject *parent)
    : QObject(parent)
    , m_snipOverlay(nullptr)
    , m_scrollTimer(new QTimer(this))
    , m_escapeTimer(new QTimer(this))
    , m_scrollSteps(0)
    , m_maxScrollSteps(20)
    , m_scrollingActive(false)
{
    m_scrollTimer->setInterval(300);
    connect(m_scrollTimer, &QTimer::timeout, this, &ScreenshotManager::scrollCaptureStep);

    // Fast polling for Escape key during scroll capture
    m_escapeTimer->setInterval(50);
    connect(m_escapeTimer, &QTimer::timeout, [this]() {
#ifdef Q_OS_WIN
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            finishScrollCapture();
        }
#endif
    });
}

ScreenshotManager::~ScreenshotManager()
{
    if (m_snipOverlay) {
        m_snipOverlay->hide();
        delete m_snipOverlay;
        m_snipOverlay = nullptr;
    }
}

void ScreenshotManager::captureFullScreen()
{
    // Hide tray menu synchronously, then wait for it to vanish
    QApplication::processEvents(QEventLoop::AllEvents, 10);
    QTimer::singleShot(120, this, [this]() {
        QScreen *screen = QApplication::primaryScreen();
        if (!screen) return;
        QPixmap px = screen->grabWindow(0);
        if (!px.isNull())
            emit screenshotTaken(px);
    });
}

void ScreenshotManager::captureSnip()
{
    cleanupOverlay();

    // Flush pending events (menu close), wait for repaint, then grab
    QApplication::processEvents(QEventLoop::AllEvents, 10);
    QTimer::singleShot(120, this, [this]() {
        QScreen *screen = QApplication::primaryScreen();
        if (!screen) return;

        m_fullScreenCapture = screen->grabWindow(0);
        if (m_fullScreenCapture.isNull()) return;

        // Create the overlay — it paints the grabbed screenshot as background
        // so the screen appears "frozen" with no visual change
        m_snipOverlay = new SnipOverlay(m_fullScreenCapture);
        connect(m_snipOverlay, &SnipOverlay::regionSelected,
                this, &ScreenshotManager::onSnipCompleted);
        connect(m_snipOverlay, &SnipOverlay::cancelled,
                this, &ScreenshotManager::onSnipCancelled);

        // Show and force immediate paint — no blank frame
        m_snipOverlay->show();
        m_snipOverlay->raise();
        m_snipOverlay->activateWindow();
        m_snipOverlay->repaint();
    });
}

void ScreenshotManager::cleanupOverlay()
{
    if (m_snipOverlay) {
        m_snipOverlay->disconnect(); // Disconnect all signals first
        m_snipOverlay->hide();
        m_snipOverlay->deleteLater();
        m_snipOverlay = nullptr;
    }
}

void ScreenshotManager::onSnipCompleted(const QRect &region)
{
    // Copy the region BEFORE destroying the overlay
    QPixmap cropped;
    if (!region.isNull() && region.isValid() && !m_fullScreenCapture.isNull()) {
        QRect safe = region.intersected(m_fullScreenCapture.rect());
        if (safe.width() > 2 && safe.height() > 2) {
            cropped = m_fullScreenCapture.copy(safe);
        }
    }

    // Now clean up overlay with a small delay to let the event loop settle
    cleanupOverlay();

    if (!cropped.isNull()) {
        m_fullScreenCapture = QPixmap();
        emit screenshotTaken(cropped);
    } else {
        m_fullScreenCapture = QPixmap();
    }
}

void ScreenshotManager::onSnipCancelled()
{
    cleanupOverlay();
    m_fullScreenCapture = QPixmap(); // free
}

// ============================================================================
// Scrolling Capture
// ============================================================================

void ScreenshotManager::captureScrolling()
{
    cleanupOverlay();

    QApplication::processEvents(QEventLoop::AllEvents, 10);
    QTimer::singleShot(120, this, [this]() {
        QScreen *screen = QApplication::primaryScreen();
        if (!screen) return;

        m_fullScreenCapture = screen->grabWindow(0);
        if (m_fullScreenCapture.isNull()) return;

        m_snipOverlay = new SnipOverlay(m_fullScreenCapture);
        connect(m_snipOverlay, &SnipOverlay::regionSelected,
                this, &ScreenshotManager::onScrollSnipCompleted);
        connect(m_snipOverlay, &SnipOverlay::cancelled,
                this, &ScreenshotManager::onSnipCancelled);
        m_snipOverlay->show();
        m_snipOverlay->raise();
        m_snipOverlay->activateWindow();
        m_snipOverlay->repaint();
    });
}

void ScreenshotManager::onScrollSnipCompleted(const QRect &region)
{
    QRect safe = region;
    if (!m_fullScreenCapture.isNull())
        safe = region.intersected(m_fullScreenCapture.rect());

    cleanupOverlay();
    m_fullScreenCapture = QPixmap(); // free — no longer needed

    if (safe.isNull() || !safe.isValid() || safe.height() < 20) return;

    m_scrollRegion = safe;
    m_scrollCaptures.clear();
    m_scrollSteps = 0;
    m_scrollingActive = true;

    // Capture first frame immediately
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QPixmap frame = screen->grabWindow(0, m_scrollRegion.x(), m_scrollRegion.y(),
                                           m_scrollRegion.width(), m_scrollRegion.height());
        if (!frame.isNull())
            m_scrollCaptures.append(frame);
    }

    QCursor::setPos(m_scrollRegion.center());
    m_scrollTimer->start();
    m_escapeTimer->start();
}

void ScreenshotManager::finishScrollCapture()
{
    m_scrollTimer->stop();
    m_escapeTimer->stop();
    m_scrollingActive = false;

    if (m_scrollCaptures.size() > 1) {
        QPixmap stitched = stitchImages(m_scrollCaptures);
        if (!stitched.isNull()) emit screenshotTaken(stitched);
    } else if (!m_scrollCaptures.isEmpty()) {
        emit screenshotTaken(m_scrollCaptures.first());
    }
    m_scrollCaptures.clear();
}

void ScreenshotManager::stopScrollCapture()
{
    if (m_scrollingActive)
        finishScrollCapture();
}

void ScreenshotManager::scrollCaptureStep()
{
    if (!m_scrollingActive || m_scrollSteps >= m_maxScrollSteps) {
        finishScrollCapture();
        return;
    }

#ifdef Q_OS_WIN
    INPUT input;
    ZeroMemory(&input, sizeof(INPUT));
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_WHEEL;
    input.mi.mouseData = (DWORD)(-WHEEL_DELTA * 6);
    SendInput(1, &input, sizeof(INPUT));
#else
    qDebug() << "Scroll simulation not available on this platform.";
    finishScrollCapture();
    return;
#endif

    QTimer::singleShot(180, this, [this]() {
        if (!m_scrollingActive) return; // stopped by escape

        QScreen *screen = QApplication::primaryScreen();
        if (!screen) return;

        QPixmap frame = screen->grabWindow(0, m_scrollRegion.x(), m_scrollRegion.y(),
                                           m_scrollRegion.width(), m_scrollRegion.height());
        if (frame.isNull()) return;

        if (!m_scrollCaptures.isEmpty()) {
            // Threshold comparison — allow small pixel differences
            QImage prev = m_scrollCaptures.last().toImage();
            QImage curr = frame.toImage();
            bool identical = true;
            int diffCount = 0;
            int sampleStep = qMax(1, curr.width() / 40);
            int totalSampled = 0;
            for (int y = 0; y < curr.height() && identical; y += 3) {
                for (int x = 0; x < curr.width(); x += sampleStep) {
                    QRgb a = prev.pixel(x, y);
                    QRgb b = curr.pixel(x, y);
                    int d = qAbs(qRed(a)-qRed(b)) + qAbs(qGreen(a)-qGreen(b)) + qAbs(qBlue(a)-qBlue(b));
                    if (d > 15) diffCount++;
                    totalSampled++;
                }
            }
            // If less than 2% of pixels differ, consider it identical
            identical = (totalSampled > 0 && diffCount * 100 / totalSampled < 2);

            if (identical) {
                // Nothing scrolled — if this is the first step, return full screen
                if (m_scrollSteps == 0) {
                    m_scrollTimer->stop();
                    m_escapeTimer->stop();
                    m_scrollingActive = false;
                    m_scrollCaptures.clear();
                    // Fall back to full screen capture
                    QPixmap fullScreen = screen->grabWindow(0);
                    if (!fullScreen.isNull())
                        emit screenshotTaken(fullScreen);
                } else {
                    finishScrollCapture();
                }
                return;
            }
        }

        m_scrollCaptures.append(frame);
        m_scrollSteps++;
    });
}

int ScreenshotManager::findOverlap(const QImage &top, const QImage &bottom) const
{
    int h = qMin(top.height(), bottom.height());
    int w = qMin(top.width(), bottom.width());
    if (w < 1 || h < 10) return 0;
    int sampleStep = qMax(1, w / 20);

    for (int overlap = h / 3; overlap >= 10; --overlap) {
        bool match = true;
        int topStartY = top.height() - overlap;
        for (int row = 0; row < overlap && match; row += 2) {
            for (int x = 0; x < w && match; x += sampleStep) {
                QRgb a = top.pixel(x, topStartY + row);
                QRgb b = bottom.pixel(x, row);
                if (qAbs(qRed(a)-qRed(b)) + qAbs(qGreen(a)-qGreen(b)) + qAbs(qBlue(a)-qBlue(b)) > 30)
                    match = false;
            }
        }
        if (match) return overlap;
    }
    return 0;
}

QPixmap ScreenshotManager::stitchImages(const QVector<QPixmap> &images) const
{
    if (images.isEmpty()) return QPixmap();
    if (images.size() == 1) return images.first();

    QVector<int> overlaps;
    int totalHeight = images.first().height();
    for (int i = 1; i < images.size(); ++i) {
        int ov = findOverlap(images[i-1].toImage(), images[i].toImage());
        overlaps.append(ov);
        totalHeight += images[i].height() - ov;
    }

    int width = images.first().width();
    if (width < 1 || totalHeight < 1) return QPixmap();

    QPixmap result(width, totalHeight);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    int y = 0;
    painter.drawPixmap(0, 0, images.first());
    y += images.first().height();
    for (int i = 1; i < images.size(); ++i) {
        y -= overlaps[i-1];
        painter.drawPixmap(0, y, images[i]);
        y += images[i].height();
    }
    return result;
}
