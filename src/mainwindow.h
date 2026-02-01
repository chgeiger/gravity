#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

class ViewportController;
class MarkerSettingsPanel;
class MarkerListPanel;
class ScenarioManager;
class QTabWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    std::unique_ptr<ViewportController> viewportController;
    std::unique_ptr<ScenarioManager> scenarioManager;
    
    QWidget *settingsPanel;
    MarkerSettingsPanel *markerSettingsPanel;
    MarkerListPanel *markerListPanel;
    QTabWidget *tabWidget;
};

#endif // MAINWINDOW_H
