#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class SphereWidget;
class QTreeWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private:
    void refreshMarkersTree();
    void onMarkerSelectionChanged();
    
    SphereWidget *sphereWidget;
    QWidget *settingsPanel;
    class MarkerSettingsPanel *markerSettingsPanel;
    QTreeWidget *markersTreeWidget;
};

#endif // MAINWINDOW_H
