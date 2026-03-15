#include "application.h"
#include "editorwindow.h"
#include "screenshotmanager.h"
#include "preferencesdialog.h"
#include "globalhotkey.h"
#include <QApplication>
#include <QMessageBox>
#include <QPainter>
#include <QStandardPaths>
#include <QTimer>

Application::Application(QObject *parent)
    : QObject(parent)
    , m_trayIcon(nullptr)
    , m_trayMenu(nullptr)
    , m_screenshotManager(new ScreenshotManager(this))
    , m_editorWindow(nullptr)
    , m_prefsDialog(nullptr)
    , m_settings(nullptr)
    , m_globalHotkey(nullptr)
    , m_snipHotkeyId(0)
    , m_fullHotkeyId(0)
    , m_scrollHotkeyId(0)
{
}

Application::~Application()
{
    delete m_editorWindow;
    delete m_prefsDialog;
}

void Application::initialize()
{
    setupSettings();
    setupTrayIcon();
    setupGlobalHotkeys();

    // Connect screenshot manager signals
    connect(m_screenshotManager, &ScreenshotManager::screenshotTaken,
            this, &Application::openEditor);
}

void Application::setupSettings()
{
    m_configPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                   + "/.snappy";
    QDir().mkpath(m_configPath);
    m_settings = new QSettings(m_configPath + "/prefs.ini", QSettings::IniFormat, this);
}

void Application::setupTrayIcon()
{
    m_trayMenu = new QMenu();

    QAction *snipAction = m_trayMenu->addAction("Snip Screenshot");
    connect(snipAction, &QAction::triggered, this, &Application::takeSnipScreenshot);

    QAction *fullAction = m_trayMenu->addAction("Full Screenshot");
    connect(fullAction, &QAction::triggered, this, &Application::takeFullScreenshot);

    QAction *scrollAction = m_trayMenu->addAction("Scrolling Capture");
    connect(scrollAction, &QAction::triggered, this, &Application::takeScrollingScreenshot);

    m_trayMenu->addSeparator();

    QAction *prefsAction = m_trayMenu->addAction("Preferences");
    connect(prefsAction, &QAction::triggered, this, &Application::showPreferences);

    QAction *aboutAction = m_trayMenu->addAction("About");
    connect(aboutAction, &QAction::triggered, this, &Application::showAbout);

    m_trayMenu->addSeparator();

    QAction *quitAction = m_trayMenu->addAction("Quit");
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    m_trayIcon = new QSystemTrayIcon(this);
    reloadIcon();
    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->setToolTip("Snappy - Screenshot Tool");
    m_trayIcon->show();

    connect(m_trayIcon, &QSystemTrayIcon::activated,
            [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) {
            // Reopen last editor if it exists, otherwise take new snip
            if (m_editorWindow && !m_editorWindow->isVisible()) {
                m_editorWindow->show();
                m_editorWindow->raise();
                m_editorWindow->activateWindow();
            } else if (!m_editorWindow) {
                takeSnipScreenshot();
            }
        }
    });
}

void Application::takeFullScreenshot()
{
    m_screenshotManager->captureFullScreen();
}

void Application::takeSnipScreenshot()
{
    m_screenshotManager->captureSnip();
}

void Application::takeScrollingScreenshot()
{
    m_screenshotManager->captureScrolling();
}

void Application::openEditor(const QPixmap &screenshot)
{
    if (screenshot.isNull() || screenshot.width() < 1 || screenshot.height() < 1)
        return;

    if (m_editorWindow) {
        m_editorWindow->hide();
        m_editorWindow->deleteLater();
        m_editorWindow = nullptr;
    }

    m_editorWindow = new EditorWindow(screenshot, m_settings);
    m_editorWindow->setAttribute(Qt::WA_DeleteOnClose, false);
    m_editorWindow->show();
    m_editorWindow->raise();
    m_editorWindow->activateWindow();
}

void Application::showPreferences()
{
    if (!m_prefsDialog) {
        m_prefsDialog = new PreferencesDialog(m_settings);
    }
    m_prefsDialog->show();
    m_prefsDialog->raise();
}

void Application::showAbout()
{
    QMessageBox::about(nullptr, "About Snappy",
        "<h2>Snappy v1.0.0</h2>"
        "<p>A lightweight screenshot tool for designers &amp; developers.</p>"
        "<p>Built with Qt " QT_VERSION_STR " and C++17.</p>");
}

QString Application::savePath() const
{
    return m_settings->value("General/SavePath",
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
}

void Application::setSavePath(const QString &path)
{
    m_settings->setValue("General/SavePath", path);
}

void Application::reloadIcon()
{
    QPixmap iconPm(":/icons/snappy.png");
    if (iconPm.isNull()) return;

    QIcon appIcon;
    for (int s : {16, 24, 32, 48, 64, 128, 256}) {
        appIcon.addPixmap(iconPm.scaled(s, s, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    m_trayIcon->setIcon(appIcon);
    qApp->setWindowIcon(appIcon);
}

void Application::setupGlobalHotkeys()
{
    m_globalHotkey = new GlobalHotkey(this);

    // Parse a shortcut string into modifiers + key
    auto parseShortcut = [this](const QString &name, const QString &def,
                                Qt::KeyboardModifiers &mods, Qt::Key &key) -> bool {
        // Clear old settings from previous versions
        QString stored = m_settings->value("Shortcuts/" + name).toString();
        if (stored.isEmpty() || stored.contains("Shift+S") ||
            stored.contains("Shift+F") || stored.contains("Shift+G")) {
            // Old default or empty — use new default
            m_settings->setValue("Shortcuts/" + name, def);
            stored = def;
        }

        QKeySequence ks(stored);
        if (ks.count() > 0) {
            int k = ks[0];
            mods = Qt::KeyboardModifiers(k & Qt::KeyboardModifierMask);
            key = Qt::Key(k & ~Qt::KeyboardModifierMask);
            return true;
        }
        return false;
    };

    Qt::KeyboardModifiers mods;
    Qt::Key key;

    if (parseShortcut("SnipScreenshot", "Ctrl+Shift+1", mods, key))
        m_snipHotkeyId = m_globalHotkey->registerHotkey(mods, key);

    if (parseShortcut("FullScreenshot", "Ctrl+Shift+3", mods, key))
        m_fullHotkeyId = m_globalHotkey->registerHotkey(mods, key);

    if (parseShortcut("ScrollCapture", "Ctrl+Shift+2", mods, key))
        m_scrollHotkeyId = m_globalHotkey->registerHotkey(mods, key);

    connect(m_globalHotkey, &GlobalHotkey::activated, [this](int id) {
        if (id == m_snipHotkeyId) takeSnipScreenshot();
        else if (id == m_fullHotkeyId) takeFullScreenshot();
        else if (id == m_scrollHotkeyId) takeScrollingScreenshot();
    });
}
