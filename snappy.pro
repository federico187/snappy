QT += core gui widgets svg

TARGET = snappy
TEMPLATE = app
CONFIG += c++17

# Windows-specific
win32 {
    RC_FILE = resources/snappy.rc
    LIBS += -luser32
}

SOURCES += \
    src/main.cpp \
    src/application.cpp \
    src/editorwindow.cpp \
    src/screenshotmanager.cpp \
    src/snipoverlay.cpp \
    src/canvasview.cpp \
    src/toolbar.cpp \
    src/preferencesdialog.cpp \
    src/annotationitems.cpp \
    src/globalhotkey.cpp

HEADERS += \
    src/application.h \
    src/editorwindow.h \
    src/screenshotmanager.h \
    src/snipoverlay.h \
    src/canvasview.h \
    src/toolbar.h \
    src/preferencesdialog.h \
    src/annotationitems.h \
    src/globalhotkey.h

RESOURCES += resources/resources.qrc
