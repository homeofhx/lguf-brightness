#include <QApplication>
#include <QIcon>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("LGUF Brightness");
    app.setWindowIcon(QIcon(":/AppIcon.svg"));
    MainWindow win;
    win.show();
    return app.exec();
}