#include "TaskApp.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    TaskApp mainWindow;
    mainWindow.setWindowTitle("Выбор режима.");
    mainWindow.resize(300, 200);
    mainWindow.show();

    return app.exec();
}

