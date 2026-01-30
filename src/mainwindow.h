#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class SphereWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private:
    SphereWidget *sphereWidget;
    QWidget *settingsPanel;
    class MarkerSettingsPanel *markerSettingsPanel;
};

#endif // MAINWINDOW_H
