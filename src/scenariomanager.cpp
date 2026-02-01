#include "scenariomanager.h"
#include "spherewidget.h"

#include <QFileDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QWidget>

ScenarioManager::ScenarioManager(SphereWidget *sphereWidget)
    : sphereWidget(sphereWidget)
{
}

void ScenarioManager::saveScenario(QWidget *parentWidget)
{
    QFileDialog dialog(parentWidget, "Szenario speichern");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter("Gravity Scenario (*.grv)");
    dialog.setDefaultSuffix("grv");
    if (!dialog.exec()) {
        return;
    }
    const QString path = dialog.selectedFiles().value(0);
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }

    const QJsonObject scenario = sphereWidget->exportScenario();
    const QJsonDocument doc(scenario);
    file.write(doc.toJson(QJsonDocument::Indented));
}

void ScenarioManager::loadScenario(QWidget *parentWidget)
{
    const QString path = QFileDialog::getOpenFileName(parentWidget, "Szenario laden", {}, "Gravity Scenario (*.grv)");
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    const QByteArray data = file.readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return;
    }

    sphereWidget->applyScenario(doc.object());
}
