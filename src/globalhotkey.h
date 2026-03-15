#ifndef GLOBALHOTKEY_H
#define GLOBALHOTKEY_H

#include <QObject>
#include <QList>
#include <QAbstractNativeEventFilter>

// Global hotkey manager.
// On Windows: RegisterHotKey/UnregisterHotKey + native event filter for WM_HOTKEY.
// On Linux: stub (integrate QHotkey or X11 for production use).

class GlobalHotkey : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit GlobalHotkey(QObject *parent = nullptr);
    ~GlobalHotkey();

    /// Register a global hotkey. Returns an ID > 0 on success, 0 on failure.
    int registerHotkey(Qt::KeyboardModifiers modifiers, Qt::Key key);

    /// Unregister by ID.
    void unregisterHotkey(int id);

    /// Unregister all.
    void unregisterAll();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
#endif

signals:
    void activated(int id);

private:
    int m_nextId;
    QList<int> m_registeredIds;
};

#endif // GLOBALHOTKEY_H
