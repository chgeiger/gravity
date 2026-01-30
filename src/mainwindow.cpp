#include "mainwindow.h"
#include "spherewidget.h"
#include "markersettingspanel.h"
#include <QColor>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QFileDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>


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

    markerSettingsPanel = new MarkerSettingsPanel(settingsPanel);
    panelLayout->addWidget(markerSettingsPanel);
    panelLayout->addStretch(1);

    connect(markerSettingsPanel, &MarkerSettingsPanel::generateRequested, this,
            [this](int count, float speedMin, float speedMax, float sizeMin, float sizeMax) {
                sphereWidget->generateMarkers(count, speedMin, speedMax, sizeMin, sizeMax);
            });

    connect(markerSettingsPanel, &MarkerSettingsPanel::animationToggled, this,
            [this](bool running) {
                sphereWidget->setAnimationEnabled(running);
            });

    connect(markerSettingsPanel, &MarkerSettingsPanel::saveRequested, this, [this]() {
        QFileDialog dialog(this, "Szenario speichern");
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        dialog.setNameFilter("Gravity Scenario (*.grv)");
        dialog.setDefaultSuffix("grv");
        if (!dialog.exec()) {
            return;
        }
        const QString path = dialog.selectedFiles().value(0);
        if (path.isEmpty()) {
            return;
        }

        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            return;
        }

        const QJsonObject scenario = sphereWidget->exportScenario();
        const QJsonDocument doc(scenario);
        file.write(doc.toJson(QJsonDocument::Indented));
    });

    connect(markerSettingsPanel, &MarkerSettingsPanel::loadRequested, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(this, "Szenario laden", {}, "Gravity Scenario (*.grv)");
        if (path.isEmpty()) {
            return;
        }

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return;
        }

        const QByteArray data = file.readAll();
        const QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) {
            return;
        }

        sphereWidget->applyScenario(doc.object());
    });

    layout->addWidget(container, 1);
    layout->addWidget(settingsPanel, 0);

    setCentralWidget(central);
}
