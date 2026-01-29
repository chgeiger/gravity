#include "mainwindow.h"
#include "spherewidget.h"
#include <QColor>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("Gravity - 3D Simulation");
    setGeometry(100, 100, 1000, 800);

    // Create Qt3D sphere widget (NOT as parent!)
    sphereWidget = new SphereWidget();
    
    // Set blue background
    sphereWidget->setBackgroundColor(QColor(25, 50, 150));
    
    // Convert Qt3DWindow to QWidget using createWindowContainer
    QWidget *container = QWidget::createWindowContainer(sphereWidget, this);
    container->setMinimumSize(800, 600);
    
    setCentralWidget(container);
}
