#include <QApplication>
#include <QFile>
#include "matforge/gui/MainWindow.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("MATForge");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("MATForge");

    // Load custom stylesheet
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(styleFile.readAll());
        styleFile.close();
    }

    matforge::gui::MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}

