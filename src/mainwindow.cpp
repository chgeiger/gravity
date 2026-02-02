#include "mainwindow.h"
#include "viewportcontroller.h"
#include "spherewidget.h"
#include "markersettingspanel.h"
#include "markerlistpanel.h"
#include "scenariomanager.h"

#include <QColor>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QTabWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("Gravity - 3D Simulation");
    setGeometry(100, 100, 1000, 800);

    // Initialize viewport controller
    viewportController = std::make_unique<ViewportController>();
    viewportController->setBackgroundColor(QColor(25, 50, 150));

    // Initialize scenario manager
    scenarioManager = std::make_unique<ScenarioManager>(viewportController->getSphereWidget());

    // Setup main layout
    auto *central = new QWidget(this);
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Add viewport
    layout->addWidget(viewportController->getContainerWidget(), 1);

    // Setup settings panel
    settingsPanel = new QWidget(central);
    settingsPanel->setFixedWidth(280);
    auto *panelLayout = new QVBoxLayout(settingsPanel);
    panelLayout->setContentsMargins(16, 16, 16, 16);
    panelLayout->setSpacing(12);
    
    auto *title = new QLabel("Einstellungen", settingsPanel);
    title->setStyleSheet("font-weight: 600; font-size: 16px;");
    panelLayout->addWidget(title);

    // Setup tab widget
    tabWidget = new QTabWidget(settingsPanel);

    // Tab 1: Marker settings
    auto *markerTab = new QWidget();
    auto *markerLayout = new QVBoxLayout(markerTab);
    markerLayout->setContentsMargins(0, 0, 0, 0);
    markerLayout->setSpacing(12);

    markerSettingsPanel = new MarkerSettingsPanel(markerTab);
    markerLayout->addWidget(markerSettingsPanel);
    markerLayout->addStretch(1);
    tabWidget->addTab(markerTab, "Marker");

    // Tab 2: Object list
    auto *objectsTab = new QWidget();
    auto *objectsLayout = new QVBoxLayout(objectsTab);
    objectsLayout->setContentsMargins(0, 0, 0, 0);
    objectsLayout->setSpacing(0);

    markerListPanel = new MarkerListPanel(viewportController->getSphereWidget(), objectsTab);
    objectsLayout->addWidget(markerListPanel);

    tabWidget->addTab(objectsTab, "Objekte");

    panelLayout->addWidget(tabWidget, 1);
    layout->addWidget(settingsPanel, 0);

    setCentralWidget(central);

    // Connect marker settings signals
    connect(markerSettingsPanel, &MarkerSettingsPanel::generateRequested, this,
            [this](int count, float speed, float size, float density) {
                viewportController->getSphereWidget()->generateMarkers(count, speed, size, density);
                markerListPanel->refreshMarkersTree();
            });

    connect(markerSettingsPanel, &MarkerSettingsPanel::animationToggled, this,
            [this](bool running) {
                viewportController->getSphereWidget()->setAnimationEnabled(running);
            });

    connect(markerSettingsPanel, &MarkerSettingsPanel::saveRequested, this,
            [this]() {
                scenarioManager->saveScenario(this);
            });

    connect(markerSettingsPanel, &MarkerSettingsPanel::loadRequested, this,
            [this]() {
                scenarioManager->loadScenario(this);
                markerListPanel->refreshMarkersTree();
            });

    connect(markerSettingsPanel, &MarkerSettingsPanel::clearAllMarkersRequested, this,
            [this]() {
                viewportController->getSphereWidget()->clearMarkers();
                markerListPanel->refreshMarkersTree();
            });

    connect(markerSettingsPanel, &MarkerSettingsPanel::zoomInRequested, this,
            [this]() {
                viewportController->getSphereWidget()->zoomIn();
            });

    connect(markerSettingsPanel, &MarkerSettingsPanel::zoomOutRequested, this,
            [this]() {
                viewportController->getSphereWidget()->zoomOut();
            });

    connect(markerSettingsPanel, &MarkerSettingsPanel::timeScaleChanged, this,
            [this](float scale) {
                viewportController->getSphereWidget()->setTimeScale(scale);
            });

    // Initial population of markers list
    markerListPanel->refreshMarkersTree();
}

MainWindow::~MainWindow() = default;
