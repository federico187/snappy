#include "globalhotkey.h"
#include <QCoreApplication>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

GlobalHotkey::GlobalHotkey(QObject *parent)
    : QObject(parent)
    , m_nextId(1)
{
    // Install native event filter so we can catch WM_HOTKEY on Windows
    QCoreApplication::instance()->installNativeEventFilter(this);
}

GlobalHotkey::~GlobalHotkey()
{
    unregisterAll();
    QCoreApplication::instance()->removeNativeEventFilter(this);
}

int GlobalHotkey::registerHotkey(Qt::KeyboardModifiers modifiers, Qt::Key key)
{
#ifdef Q_OS_WIN
    // Convert Qt modifiers to Windows MOD_ flags
    unsigned int winMods = 0;
    if (modifiers & Qt::ControlModifier) winMods |= MOD_CONTROL;
    if (modifiers & Qt::ShiftModifier)   winMods |= MOD_SHIFT;
    if (modifiers & Qt::AltModifier)     winMods |= MOD_ALT;
    if (modifiers & Qt::MetaModifier)    winMods |= MOD_WIN;

    // Convert Qt key to Windows virtual key code
    unsigned int vk = 0;
    if (key >= Qt::Key_A && key <= Qt::Key_Z) {
        vk = key; // Qt::Key_A == 0x41 == 'A', same as Windows VK
    } else if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        vk = key;
    } else if (key >= Qt::Key_F1 && key <= Qt::Key_F24) {
        vk = VK_F1 + (key - Qt::Key_F1);
    } else if (key == Qt::Key_Print) {
        vk = VK_SNAPSHOT;
    } else if (key == Qt::Key_Space) {
        vk = VK_SPACE;
    } else if (key == Qt::Key_Return || key == Qt::Key_Enter) {
        vk = VK_RETURN;
    } else if (key == Qt::Key_Escape) {
        vk = VK_ESCAPE;
    } else {
        qWarning() << "GlobalHotkey: Unsupported key" << key;
        return 0;
    }

    int id = m_nextId++;
    if (RegisterHotKey(nullptr, id, winMods | MOD_NOREPEAT, vk)) {
        m_registeredIds.append(id);
        qDebug() << "GlobalHotkey: Registered hotkey id" << id;
        return id;
    } else {
        qWarning() << "GlobalHotkey: Failed to register hotkey id" << id
                    << "error:" << GetLastError();
        return 0;
    }
#else
    // Linux/macOS stub
    Q_UNUSED(modifiers);
    Q_UNUSED(key);
    qDebug() << "GlobalHotkey: Stub (not implemented on this platform). "
                "Use QHotkey or X11 integration for production.";
    int id = m_nextId++;
    m_registeredIds.append(id);
    return id;
#endif
}

void GlobalHotkey::unregisterHotkey(int id)
{
#ifdef Q_OS_WIN
    UnregisterHotKey(nullptr, id);
#endif
    m_registeredIds.removeAll(id);
}

void GlobalHotkey::unregisterAll()
{
    for (int id : m_registeredIds) {
#ifdef Q_OS_WIN
        UnregisterHotKey(nullptr, id);
#endif
    }
    m_registeredIds.clear();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool GlobalHotkey::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
#else
bool GlobalHotkey::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
#endif
{
    Q_UNUSED(result);

#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
        MSG *msg = static_cast<MSG *>(message);
        if (msg->message == WM_HOTKEY) {
            int id = static_cast<int>(msg->wParam);
            if (m_registeredIds.contains(id)) {
                emit activated(id);
                return true;
            }
        }
    }
#else
    Q_UNUSED(eventType);
    Q_UNUSED(message);
#endif

    return false;
}
