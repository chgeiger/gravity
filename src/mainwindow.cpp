#include "mainwindow.h"
#include "spherewidget.h"
#include <QColor>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>


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

    auto *central = new QWidget(this);
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    settingsPanel = new QWidget(central);
    settingsPanel->setFixedWidth(280);
 //   settingsPanel->setStyleSheet("background-color: #1f1f1f; color: #e0e0e0;");

    auto *panelLayout = new QVBoxLayout(settingsPanel);
    panelLayout->setContentsMargins(16, 16, 16, 16);
    panelLayout->setSpacing(12);
    auto *title = new QLabel("Einstellungen", settingsPanel);
    title->setStyleSheet("font-weight: 600; font-size: 16px;");
    panelLayout->addWidget(title);

    auto *markerGroup = new QGroupBox("Marker erzeugen", settingsPanel);
    auto *markerForm = new QFormLayout(markerGroup);
    markerForm->setLabelAlignment(Qt::AlignLeft);
    markerForm->setFormAlignment(Qt::AlignTop);

    auto *countEdit = new QLineEdit(markerGroup);
    countEdit->setText("8");
    countEdit->setPlaceholderText("z. B. 8");

    auto *speedMinEdit = new QLineEdit(markerGroup);
    speedMinEdit->setText("0.2");
    speedMinEdit->setPlaceholderText("z. B. 0.2");
    auto *speedMaxEdit = new QLineEdit(markerGroup);
    speedMaxEdit->setText("0.8");
    speedMaxEdit->setPlaceholderText("z. B. 0.8");

    auto *sizeMinEdit = new QLineEdit(markerGroup);
    sizeMinEdit->setText("0.05");
    sizeMinEdit->setPlaceholderText("z. B. 0.05");
    auto *sizeMaxEdit = new QLineEdit(markerGroup);
    sizeMaxEdit->setText("0.15");
    sizeMaxEdit->setPlaceholderText("z. B. 0.2");

    markerForm->addRow("Anzahl", countEdit);
    markerForm->addRow("Geschwindigkeit min", speedMinEdit);
    markerForm->addRow("Geschwindigkeit max", speedMaxEdit);
    markerForm->addRow("Größe min", sizeMinEdit);
    markerForm->addRow("Größe max", sizeMaxEdit);

    panelLayout->addWidget(markerGroup);

    auto *generateButton = new QPushButton("Marker erzeugen", settingsPanel);
    generateButton->setMinimumHeight(32);
    panelLayout->addWidget(generateButton);
    panelLayout->addStretch(1);

    layout->addWidget(container, 1);
    layout->addWidget(settingsPanel, 0);

    setCentralWidget(central);
}
