#ifndef SCENARIOMANAGER_H
#define SCENARIOMANAGER_H

#include <QString>
#include <QJsonObject>

class SphereWidget;
class QWidget;

/**
 * @brief ScenarioManager - Verwaltung von Szenarios-Operationen
 * 
 * Verantwortlichkeiten:
 * - Speichern von Szenarien in JSON-Format (.grv-Dateien)
 * - Laden von gespeicherten Szenarien aus JSON-Dateien
 * - Bereitstellung von Dateidialog-Interaktionen fuer den Benutzer
 * - Serialisierung und Deserialisierung von Simulations-Szenarien
 */
class ScenarioManager {
public:
    explicit ScenarioManager(SphereWidget *sphereWidget);

    void saveScenario(QWidget *parentWidget);
    void loadScenario(QWidget *parentWidget);

private:
    SphereWidget *sphereWidget;
};

#endif // SCENARIOMANAGER_H
