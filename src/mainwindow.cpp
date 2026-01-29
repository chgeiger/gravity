#include "mainwindow.h"
#include "spherewidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("Gravity - 3D Sphere");
    setGeometry(100, 100, 800, 600);

    sphereWidget = new SphereWidget(this);
    setCentralWidget(sphereWidget);
}
