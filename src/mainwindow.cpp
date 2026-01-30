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
#include <QTabWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>


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
    auto *panelLayout = new QVBoxLayout(settingsPanel);
    panelLayout->setContentsMargins(16, 16, 16, 16);
    panelLayout->setSpacing(12);
    auto *title = new QLabel("Einstellungen", settingsPanel);
    title->setStyleSheet("font-weight: 600; font-size: 16px;");
    panelLayout->addWidget(title);

    auto *tabWidget = new QTabWidget(settingsPanel);
    
    // Tab 1: Marker
    auto *markerTab = new QWidget();
    auto *markerLayout = new QVBoxLayout(markerTab);
    markerLayout->setContentsMargins(0, 0, 0, 0);
    markerLayout->setSpacing(12);
    
    markerSettingsPanel = new MarkerSettingsPanel(markerTab);
    markerLayout->addWidget(markerSettingsPanel);
    markerLayout->addStretch(1);
    tabWidget->addTab(markerTab, "Marker");
    
    // Tab 2: Objekte
    auto *objectsTab = new QWidget();
    auto *objectsLayout = new QVBoxLayout(objectsTab);
    objectsLayout->setContentsMargins(0, 0, 0, 0);
    objectsLayout->setSpacing(0);
    
    markersTreeWidget = new QTreeWidget(objectsTab);
    markersTreeWidget->setHeaderLabel("Marker");
    markersTreeWidget->setColumnCount(1);
    objectsLayout->addWidget(markersTreeWidget);
    tabWidget->addTab(objectsTab, "Objekte");
    
    panelLayout->addWidget(tabWidget, 1);

    connect(markerSettingsPanel, &MarkerSettingsPanel::generateRequested, this,
            [this](int count, float speedMin, float speedMax, float sizeMin, float sizeMax) {
                sphereWidget->generateMarkers(count, speedMin, speedMax, sizeMin, sizeMax);
                refreshMarkersTree();
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

    connect(markerSettingsPanel, &MarkerSettingsPanel::clearAllMarkersRequested, this, [this]() {
        sphereWidget->clearMarkers();
        refreshMarkersTree();
    });

    connect(markerSettingsPanel, &MarkerSettingsPanel::zoomInRequested, this, [this]() {
        sphereWidget->zoomIn();
    });

    connect(markerSettingsPanel, &MarkerSettingsPanel::zoomOutRequested, this, [this]() {
        sphereWidget->zoomOut();
    });

    connect(markersTreeWidget, &QTreeWidget::itemClicked, this, &MainWindow::onMarkerSelectionChanged);

    layout->addWidget(container, 1);
    layout->addWidget(settingsPanel, 0);

    setCentralWidget(central);
    
    // Initial population of markers tree
    refreshMarkersTree();
}

void MainWindow::refreshMarkersTree()
{
    markersTreeWidget->clear();
    
    const auto markers = sphereWidget->getMarkersInfo();
    
    for (const auto &markerInfo : markers) {
        auto *markerItem = new QTreeWidgetItem();
        markerItem->setText(0, QString("Marker %1").arg(markerInfo.index + 1));
        
        auto *radiusItem = new QTreeWidgetItem(markerItem);
        radiusItem->setText(0, QString("Radius: %1").arg(markerInfo.radius, 0, 'f', 3));
        
        auto *colorItem = new QTreeWidgetItem(markerItem);
        colorItem->setText(0, QString("Farbe: RGB(%1, %2, %3)")
            .arg(markerInfo.color.red())
            .arg(markerInfo.color.green())
            .arg(markerInfo.color.blue()));
        
        auto *posItem = new QTreeWidgetItem(markerItem);
        posItem->setText(0, QString("Position: (%1, %2, %3)")
            .arg(markerInfo.position.x(), 0, 'f', 3)
            .arg(markerInfo.position.y(), 0, 'f', 3)
            .arg(markerInfo.position.z(), 0, 'f', 3));
        
        auto *velItem = new QTreeWidgetItem(markerItem);
        velItem->setText(0, QString("Geschwindigkeit: (%1, %2, %3)")
            .arg(markerInfo.velocity.x(), 0, 'f', 3)
            .arg(markerInfo.velocity.y(), 0, 'f', 3)
            .arg(markerInfo.velocity.z(), 0, 'f', 3));
        
        markersTreeWidget->addTopLevelItem(markerItem);
        markerItem->setExpanded(false);
    }
}

void MainWindow::onMarkerSelectionChanged()
{
    qDebug() << "onMarkerSelectionChanged called";
    
    QTreeWidgetItem *selectedItem = markersTreeWidget->currentItem();
    
    if (!selectedItem) {
        qDebug() << "No item selected";
        sphereWidget->clearHighlightedMarker();
        return;
    }
    
    qDebug() << "Selected item:" << selectedItem->text(0);
    
    // Finde den Parent-Item (Top-Level Item)
    QTreeWidgetItem *markerItem = selectedItem;
    while (markerItem->parent() != nullptr) {
        markerItem = markerItem->parent();
    }
    
    qDebug() << "Marker item:" << markerItem->text(0);
    
    // Finde den Index des Markers
    int markerIndex = markersTreeWidget->indexOfTopLevelItem(markerItem);
    qDebug() << "Marker index:" << markerIndex;
    
    if (markerIndex >= 0) {
        sphereWidget->highlightMarker(markerIndex);
    }
}
