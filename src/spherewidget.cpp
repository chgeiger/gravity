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

SphereWidget::SphereWidget()
    : Qt3DExtras::Qt3DWindow(), rotationAngle(0.0f), sphereTransform(nullptr), cameraController(nullptr)
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
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SphereWidget::updateFrame);
    timer->start(16); // ~60 FPS
}

Qt3DCore::QEntity *SphereWidget::createScene()
{
    qDebug() << "Creating Qt3D scene...";
    
    // Root entity
    auto *rootEntity = new Qt3DCore::QEntity();
    
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

    constexpr float sphereRadius = 1.0f;
    constexpr float markerRadius = 0.3f;

    auto fromLatLon = [](float latDeg, float lonDeg) {
        const float latRad = qDegreesToRadians(latDeg);
        const float lonRad = qDegreesToRadians(lonDeg);
        const float cosLat = qCos(latRad);
        return QVector3D(
            cosLat * qCos(lonRad),
            qSin(latRad),
            cosLat * qSin(lonRad)
        ).normalized();
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

    auto *markerA = new SurfaceMarker(rootEntity, sphereRadius, markerRadius, QColor(220, 60, 60));
    const QVector3D posA = fromLatLon(25.0f, 10.0f);
    markerA->setSphericalPosition(25.0f, 10.0f);
    markers.append({markerA, posA, tangentDirection(posA, QVector3D(1.0f, 0.2f, 0.5f)) * 0.6f});

    auto *markerB = new SurfaceMarker(rootEntity, sphereRadius, markerRadius, QColor(60, 200, 120));
    const QVector3D posB = fromLatLon(-10.0f, 120.0f);
    markerB->setSphericalPosition(-10.0f, 120.0f);
    markers.append({markerB, posB, tangentDirection(posB, QVector3D(-0.2f, 1.0f, 0.3f)) * 0.5f});

    auto *markerC = new SurfaceMarker(rootEntity, sphereRadius, markerRadius, QColor(80, 120, 220));
    const QVector3D posC = fromLatLon(45.0f, -60.0f);
    markerC->setSphericalPosition(45.0f, -60.0f);
    markers.append({markerC, posC, tangentDirection(posC, QVector3D(0.4f, 0.1f, -1.0f)) * 0.55f});
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
