#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class SphereWidget;
class QTreeWidget;
class QCheckBox;
class QComboBox;

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
    QCheckBox *markerActionCheckBox;
    QComboBox *markerSelectionCombo;
};

#endif // MAINWINDOW_H
