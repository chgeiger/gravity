#ifndef MARKERLISTPANEL_H
#define MARKERLISTPANEL_H

#include <QWidget>

class QListWidget;
class QCheckBox;
class QComboBox;
class QGroupBox;
class SphereWidget;
class EditablePropertyWidget;

/**
 * @brief MarkerListPanel - Verwaltet Marker-Uebersicht und Verfolgungssteuerung
 * 
 * Verantwortlichkeiten:
 * - Anzeige aller Marker in einer Baumansicht mit Details (Radius, Farbe, Position, Geschwindigkeit)
 * - Verwaltung der Marker-Verfolgungssteuerung (aktiv/inaktiv und Marker-Auswahl)
 * - Aktualisierung der Baumansicht nach Generierung oder Aenderung von Markern
 * - Handling von Marker-Auswahl durch die Baumansicht
 */
class MarkerListPanel : public QWidget {
    Q_OBJECT

public:
    explicit MarkerListPanel(SphereWidget *sphereWidget, QWidget *parent = nullptr);

    void refreshMarkersTree();

private slots:
    void onMarkerSelectionChanged();
    void updateSelectedMarkerProperties();

private:
    SphereWidget *sphereWidget;
    QListWidget *markersListWidget;
    QCheckBox *markerActionCheckBox;
    QComboBox *markerSelectionCombo;
    
    // Eigenschaften des ausgewählten Markers
    QGroupBox *selectedMarkerGroup;
    EditablePropertyWidget *selectedRadiusWidget;
    EditablePropertyWidget *selectedDensityWidget;
    EditablePropertyWidget *selectedVelocityWidget;
    QVector<int> selectedMarkerIndices;
};

#endif // MARKERLISTPANEL_H
