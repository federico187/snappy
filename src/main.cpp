// main.cpp - Snappy
// A lightweight screenshot tool for designers & developers
// Inspired by Pawxel (https://github.com/yeahitsjan/pawxel)

#include "application.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Snappy - Screenshot Tool");
    app.setApplicationDisplayName("Snappy - Screenshot Tool");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("snappy");
    app.setQuitOnLastWindowClosed(false);

    Application snappy;
    snappy.initialize();

    return app.exec();
}
