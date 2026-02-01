#ifndef MARKERLISTPANEL_H
#define MARKERLISTPANEL_H

#include <QWidget>

class QTreeWidget;
class QCheckBox;
class QComboBox;
class SphereWidget;

class MarkerListPanel : public QWidget {
    Q_OBJECT

public:
    explicit MarkerListPanel(SphereWidget *sphereWidget, QWidget *parent = nullptr);

    void refreshMarkersTree();

private slots:
    void onMarkerSelectionChanged();

private:
    SphereWidget *sphereWidget;
    QTreeWidget *markersTreeWidget;
    QCheckBox *markerActionCheckBox;
    QComboBox *markerSelectionCombo;
};

#endif // MARKERLISTPANEL_H
