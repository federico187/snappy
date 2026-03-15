#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QSettings>
#include <QDir>

class EditorWindow;
class ScreenshotManager;
class PreferencesDialog;
class GlobalHotkey;

class Application : public QObject
{
    Q_OBJECT

public:
    explicit Application(QObject *parent = nullptr);
    ~Application();

    void initialize();
    void reloadIcon();

    // Settings
    QString savePath() const;
    void setSavePath(const QString &path);

public slots:
    void takeFullScreenshot();
    void takeSnipScreenshot();
    void takeScrollingScreenshot();
    void showPreferences();
    void showAbout();

private:
    void setupTrayIcon();
    void setupSettings();
    void setupGlobalHotkeys();
    void openEditor(const QPixmap &screenshot);

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    ScreenshotManager *m_screenshotManager;
    EditorWindow *m_editorWindow;
    PreferencesDialog *m_prefsDialog;
    QSettings *m_settings;
    GlobalHotkey *m_globalHotkey;
    QString m_configPath;
    int m_snipHotkeyId;
    int m_fullHotkeyId;
    int m_scrollHotkeyId;
};

#endif // APPLICATION_H
