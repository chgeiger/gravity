#include "spherewidget.h"

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPointLight>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QOrbitCameraController>
#include <QTimer>
#include <QDebug>
#include <QtMath>
#include <QRandomGenerator>
#include <algorithm>
#include <QJsonArray>

SphereWidget::SphereWidget()
    : Qt3DExtras::Qt3DWindow(),
      rotationAngle(0.0f),
      sphereTransform(nullptr),
      cameraController(nullptr),
    rootEntity(nullptr),
    animationTimer(nullptr),
    animationEnabled(true)
{
    setTitle("Gravity Simulator - Qt3D");

    frameTimer.start();
    lastFrameMs = frameTimer.elapsed();
    
    // Setup camera first
    auto *camera = this->camera();
    camera->setPosition(QVector3D(0, 0, 3.5f));
    camera->setViewCenter(QVector3D(0, 0, 0));
    camera->setUpVector(QVector3D(0, 1, 0));
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    
    // Create scene
    Qt3DCore::QEntity *scene = createScene();
    
    // Set root entity
    setRootEntity(scene);

    // Mouse navigation: orbit camera controller
    cameraController = new Qt3DExtras::QOrbitCameraController(scene);
    cameraController->setLinearSpeed(50.0f);
    cameraController->setLookSpeed(180.0f);
    cameraController->setCamera(camera);
    
    // Setup animation timer
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &SphereWidget::updateFrame);
    animationTimer->start(16); // ~60 FPS
}

Qt3DCore::QEntity *SphereWidget::createScene()
{
    qDebug() << "Creating Qt3D scene...";
    
    // Root entity
    rootEntity = new Qt3DCore::QEntity();
    
    // Create lighting and sphere
    createLighting(rootEntity);
    createSphere(rootEntity);
    createMarkers(rootEntity);
    
    qDebug() << "Scene created successfully";
    return rootEntity;
}

void SphereWidget::createMarkers(Qt3DCore::QEntity *rootEntity)
{
    if (!rootEntity) {
        qWarning() << "rootEntity is null in createMarkers!";
        return;
    }

    generateMarkers(8, 0.2f, 0.8f, 0.05f, 0.15f);
}

void SphereWidget::clearMarkers()
{
    for (auto &state : markers) {
        if (state.marker) {
            if (auto *entity = state.marker->entity()) {
                delete entity;
            }
            delete state.marker;
        }
    }
    markers.clear();
}

void SphereWidget::generateMarkers(int count, float speedMin, float speedMax, float sizeMin, float sizeMax)
{
    if (!rootEntity) {
        return;
    }

    clearMarkers();

    if (count <= 0) {
        return;
    }

    if (speedMax < speedMin) {
        std::swap(speedMin, speedMax);
    }
    if (sizeMax < sizeMin) {
        std::swap(sizeMin, sizeMax);
    }

    constexpr float sphereRadius = 1.0f;
    auto *rng = QRandomGenerator::global();

    auto randRange = [rng](double minValue, double maxValue) {
        return minValue + (maxValue - minValue) * rng->generateDouble();
    };

    auto randomUnitVector = [&]() {
        const float z = static_cast<float>(randRange(-1.0, 1.0));
        const float t = static_cast<float>(randRange(0.0, 2.0 * M_PI));
        const float r = qSqrt(qMax(0.0f, 1.0f - z * z));
        return QVector3D(r * qCos(t), z, r * qSin(t)).normalized();
    };

    auto tangentDirection = [](const QVector3D &pos, const QVector3D &dir) {
        QVector3D tangent = dir - QVector3D::dotProduct(dir, pos) * pos;
        if (tangent.lengthSquared() < 1e-6f) {
            tangent = QVector3D::crossProduct(pos, QVector3D(0.0f, 1.0f, 0.0f));
            if (tangent.lengthSquared() < 1e-6f) {
                tangent = QVector3D::crossProduct(pos, QVector3D(1.0f, 0.0f, 0.0f));
            }
        }
        return tangent.normalized();
    };

    for (int i = 0; i < count; ++i) {
        const QVector3D position = randomUnitVector();
        const float latDeg = qRadiansToDegrees(qAsin(position.y()));
        const float lonDeg = qRadiansToDegrees(qAtan2(position.z(), position.x()));

        const float markerRadius = static_cast<float>(randRange(sizeMin, sizeMax));
        const float speed = static_cast<float>(randRange(speedMin, speedMax));

        const QColor color = QColor::fromHsv(rng->bounded(0, 360), 200, 230);
        auto *marker = new SurfaceMarker(rootEntity, sphereRadius, markerRadius, color);
        marker->setSphericalPosition(latDeg, lonDeg);

        const QVector3D dir = randomUnitVector();
        const QVector3D velocity = tangentDirection(position, dir) * speed;

        markers.append({marker, position, velocity, markerRadius, color});
    }
}

QJsonObject SphereWidget::exportScenario() const
{
    QJsonObject root;
    root["version"] = 1;
    root["animationEnabled"] = animationEnabled;
    root["sphereRadius"] = 1.0;

    QJsonArray markerArray;

    for (const auto &state : markers) {
        QJsonObject markerObj;
        markerObj["radius"] = state.radius;
        markerObj["color"] = QJsonArray{state.color.red(), state.color.green(), state.color.blue()};
        markerObj["position"] = QJsonArray{state.position.x(), state.position.y(), state.position.z()};
        markerObj["velocity"] = QJsonArray{state.velocity.x(), state.velocity.y(), state.velocity.z()};
        markerArray.append(markerObj);
    }

    root["markers"] = markerArray;
    return root;
}

bool SphereWidget::applyScenario(const QJsonObject &scenario)
{
    if (!scenario.contains("markers") || !scenario["markers"].isArray()) {
        return false;
    }

    const QJsonArray markerArray = scenario["markers"].toArray();
    clearMarkers();

    constexpr float sphereRadius = 1.0f;

    for (const auto &entry : markerArray) {
        if (!entry.isObject()) {
            continue;
        }

        const QJsonObject markerObj = entry.toObject();
        const QJsonArray colorArr = markerObj["color"].toArray();
        const QJsonArray posArr = markerObj["position"].toArray();
        const QJsonArray velArr = markerObj["velocity"].toArray();

        if (colorArr.size() != 3 || posArr.size() != 3 || velArr.size() != 3) {
            continue;
        }

        const float radius = static_cast<float>(markerObj["radius"].toDouble(0.1));
        const QColor color(
            colorArr[0].toInt(255),
            colorArr[1].toInt(255),
            colorArr[2].toInt(255)
        );

        const QVector3D position(
            static_cast<float>(posArr[0].toDouble()),
            static_cast<float>(posArr[1].toDouble()),
            static_cast<float>(posArr[2].toDouble())
        );

        const QVector3D velocity(
            static_cast<float>(velArr[0].toDouble()),
            static_cast<float>(velArr[1].toDouble()),
            static_cast<float>(velArr[2].toDouble())
        );

        auto *marker = new SurfaceMarker(rootEntity, sphereRadius, radius, color);
        const QVector3D posNorm = position.normalized();
        const float latDeg = qRadiansToDegrees(qAsin(posNorm.y()));
        const float lonDeg = qRadiansToDegrees(qAtan2(posNorm.z(), posNorm.x()));
        marker->setSphericalPosition(latDeg, lonDeg);

        markers.append({marker, posNorm, velocity, radius, color});
    }

    const bool animEnabled = scenario["animationEnabled"].toBool(true);
    setAnimationEnabled(animEnabled);
    return true;
}

void SphereWidget::createLighting(Qt3DCore::QEntity *rootEntity)
{
    if (!rootEntity) {
        qWarning() << "rootEntity is null in createLighting!";
        return;
    }
    
    // Main light
    auto *lightEntity = new Qt3DCore::QEntity(rootEntity);
    auto *light = new Qt3DRender::QPointLight(lightEntity);
    light->setIntensity(1.0f);
    light->setColor(QColor(255, 255, 255));
    
    auto *lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation(QVector3D(3.0f, 3.0f, 3.0f));
    
    lightEntity->addComponent(light);
    lightEntity->addComponent(lightTransform);
    
    qDebug() << "Lighting created";
}

void SphereWidget::createSphere(Qt3DCore::QEntity *rootEntity)
{
    if (!rootEntity) {
        qWarning() << "rootEntity is null in createSphere!";
        return;
    }
    
    // Sphere entity
    auto *sphereEntity = new Qt3DCore::QEntity(rootEntity);
    
    // Sphere mesh
    auto *sphereMesh = new Qt3DExtras::QSphereMesh();
    sphereMesh->setRadius(1.0f);
    sphereMesh->setSlices(50);
    sphereMesh->setRings(50);
    
    // Phong material - orange sphere
    auto *material = new Qt3DExtras::QPhongMaterial();
    material->setDiffuse(QColor(255, 165, 0)); // Orange
    material->setSpecular(QColor(200, 200, 200));
    material->setShininess(64.0f);
    material->setAmbient(QColor(100, 80, 60));
    
    // Transform
    sphereTransform = new Qt3DCore::QTransform();
    sphereTransform->setTranslation(QVector3D(0, 0, 0));
    sphereTransform->setScale(1.0f);
    
    // Add components
    sphereEntity->addComponent(sphereMesh);
    sphereEntity->addComponent(material);
    sphereEntity->addComponent(sphereTransform);
    
    qDebug() << "Sphere created";
}

void SphereWidget::updateFrame()
{
    const qint64 nowMs = frameTimer.elapsed();
    const float deltaSeconds = qMax(0.0f, (nowMs - lastFrameMs) / 1000.0f);
    lastFrameMs = nowMs;

    updateMarkers(deltaSeconds);

    if (sphereTransform) {
        rotationAngle += 0.5f; // Rotate 0.5 degrees per frame
        if (rotationAngle >= 360.0f) {
            rotationAngle -= 360.0f;
        }
        
        auto rotation = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), rotationAngle);
        sphereTransform->setRotation(rotation);
    }
}

void SphereWidget::updateMarkers(float deltaSeconds)
{
    if (deltaSeconds <= 0.0f) {
        return;
    }

    for (auto &state : markers) {
        const QVector3D position = state.position.normalized();
        QVector3D velocity = state.velocity;
        velocity -= QVector3D::dotProduct(velocity, position) * position;

        const float speed = velocity.length();
        if (speed > 1e-6f) {
            const QVector3D axis = QVector3D::crossProduct(position, velocity).normalized();
            const float angleRad = (speed * deltaSeconds);
            const QQuaternion rotation = QQuaternion::fromAxisAndAngle(axis, qRadiansToDegrees(angleRad));

            state.position = rotation.rotatedVector(position).normalized();
            state.velocity = rotation.rotatedVector(velocity);
        }

        const float latDeg = qRadiansToDegrees(qAsin(state.position.y()));
        const float lonDeg = qRadiansToDegrees(qAtan2(state.position.z(), state.position.x()));
        state.marker->setSphericalPosition(latDeg, lonDeg);
    }
}

void SphereWidget::setAnimationEnabled(bool enabled)
{
    if (animationEnabled == enabled) {
        return;
    }

    animationEnabled = enabled;
    if (animationTimer) {
        if (animationEnabled) {
            lastFrameMs = frameTimer.elapsed();
            animationTimer->start(16);
        } else {
            animationTimer->stop();
        }
    }
}
