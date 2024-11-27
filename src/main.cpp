#include "headers/MainWindow.h"
#include <QApplication>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow mainWindow;
    cout << "Runnnig ..." << endl;
    mainWindow.show();  // Ensure this line is present to show the MainWindow
    return app.exec();
}
