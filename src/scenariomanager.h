#ifndef SCENARIOMANAGER_H
#define SCENARIOMANAGER_H

#include <QString>
#include <QJsonObject>

class SphereWidget;
class QWidget;

class ScenarioManager {
public:
    explicit ScenarioManager(SphereWidget *sphereWidget);

    void saveScenario(QWidget *parentWidget);
    void loadScenario(QWidget *parentWidget);

private:
    SphereWidget *sphereWidget;
};

#endif // SCENARIOMANAGER_H
