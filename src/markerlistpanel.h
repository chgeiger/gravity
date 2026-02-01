#ifndef MARKERLISTPANEL_H
#define MARKERLISTPANEL_H

#include <QWidget>

class QTreeWidget;
class QCheckBox;
class QComboBox;
class SphereWidget;

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

private:
    SphereWidget *sphereWidget;
    QTreeWidget *markersTreeWidget;
    QCheckBox *markerActionCheckBox;
    QComboBox *markerSelectionCombo;
};

#endif // MARKERLISTPANEL_H
