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
      sphereTransform(nullptr),
      cameraController(nullptr),
    rootEntity(nullptr),
    animationTimer(nullptr),
    animationEnabled(true),
    highlightedMarkerIndex(-1),
    selectedMarkerIndex(-1),
    followMarkerEnabled(false),
    followMarkerDistance(3.5f)
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

    generateMarkers(8, 0.5f, 0.1f, 1.0f);
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
    highlightedMarkerIndex = -1;
    selectedMarkerIndex = -1;
}

void SphereWidget::generateMarkers(int count, float speed, float size, float density)
{
    if (!rootEntity) {
        return;
    }

    if (count <= 0) {
        return;
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

    const QColor baseColor(120, 190, 255);

    for (int i = 0; i < count; ++i) {
        const QVector3D position = randomUnitVector();
        const float latDeg = qRadiansToDegrees(qAsin(position.y()));
        const float lonDeg = qRadiansToDegrees(qAtan2(position.z(), position.x()));

        auto *marker = new SurfaceMarker(rootEntity, sphereRadius, size, baseColor);
        marker->setSphericalPosition(latDeg, lonDeg);

        const QVector3D dir = randomUnitVector();
        const QVector3D velocity = tangentDirection(position, dir) * speed;

        markers.append({marker, position, velocity, size, density, baseColor});
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
        markerObj["density"] = state.density;
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
        const float density = static_cast<float>(markerObj["density"].toDouble(1.0));
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

        markers.append({marker, posNorm, velocity, radius, density, color});
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
    
    // Kamera dem Marker folgen lassen
    if (followMarkerEnabled && selectedMarkerIndex >= 0 && selectedMarkerIndex < markers.size()) {
        auto *cam = camera();
        const QVector3D markerPos = markers[selectedMarkerIndex].position.normalized();
        
        // Neue Kamera-Position: in Richtung des Markers, mit konfigurierter Distanz
        const QVector3D newCamPos = markerPos * followMarkerDistance;
        
        // Interpolation für sanfte Bewegung
        const QVector3D currentPos = cam->position();
        const QVector3D smoothPos = currentPos + (newCamPos - currentPos) * 0.1f;
        
        cam->setPosition(smoothPos);
        cam->setViewCenter(QVector3D(0, 0, 0));
        
        // Up-Vektor anpassen (senkrecht zur Marker-Position)
        QVector3D up = QVector3D(0, 1, 0);
        up -= QVector3D::dotProduct(up, markerPos) * markerPos;
        if (up.lengthSquared() > 1e-6f) {
            cam->setUpVector(up.normalized());
        }
    }
}

void SphereWidget::updateMarkers(float deltaSeconds)
{
    if (deltaSeconds <= 0.0f) {
        return;
    }

    constexpr float sphereRadius = 1.0f;
    constexpr float gravityConstant = 10.0f;
    const float epsilon = 1e-4f;

    QVector<QVector3D> accelerations(markers.size(), QVector3D(0.0f, 0.0f, 0.0f));

    for (int i = 0; i < markers.size(); ++i) {
        for (int j = i + 1; j < markers.size(); ++j) {
            const QVector3D pa = markers[i].position.normalized();
            const QVector3D pb = markers[j].position.normalized();

            const float dot = qBound(-1.0f, QVector3D::dotProduct(pa, pb), 1.0f);
            const float angle = qAcos(dot);
            const float arc = qMax(angle * sphereRadius, epsilon);
            const float otherArc = qMax(static_cast<float>(2.0 * M_PI) * sphereRadius - arc, epsilon);

            QVector3D ti = pb - QVector3D::dotProduct(pb, pa) * pa;
            QVector3D tj = pa - QVector3D::dotProduct(pa, pb) * pb;

            if (ti.lengthSquared() < epsilon || tj.lengthSquared() < epsilon) {
                continue;
            }

            ti.normalize();
            tj.normalize();

            const float mi = markers[i].density * markers[i].radius * markers[i].radius * markers[i].radius;
            const float mj = markers[j].density * markers[j].radius * markers[j].radius * markers[j].radius;

            const float forceMagnitude = gravityConstant * mi * mj * ((1.0f / (arc * arc)) - (1.0f / (otherArc * otherArc)));

            accelerations[i] += (forceMagnitude / mi) * ti;
            accelerations[j] += (forceMagnitude / mj) * tj;
        }
    }

    for (int i = 0; i < markers.size(); ++i) {
        auto &state = markers[i];
        const QVector3D position = state.position.normalized();
        QVector3D velocity = state.velocity + accelerations.value(i) * deltaSeconds;
        velocity -= QVector3D::dotProduct(velocity, position) * position;

        const float speed = velocity.length();
        if (speed > 1e-6f) {
            const QVector3D axis = QVector3D::crossProduct(position, velocity).normalized();
            const float angleRad = (speed * deltaSeconds);
            const QQuaternion rotation = QQuaternion::fromAxisAndAngle(axis, qRadiansToDegrees(angleRad));

            state.position = rotation.rotatedVector(position).normalized();
            state.velocity = rotation.rotatedVector(velocity);
        }

    }

    const QColor baseColor(120, 190, 255);
    const QColor hitColor(255, 220, 80);

    QVector<bool> colliding(markers.size(), false);
    handleCollisions(colliding);

    for (auto &state : markers) {
        const float latDeg = qRadiansToDegrees(qAsin(state.position.y()));
        const float lonDeg = qRadiansToDegrees(qAtan2(state.position.z(), state.position.x()));
        state.marker->setSphericalPosition(latDeg, lonDeg);
    }

    for (int i = 0; i < markers.size(); ++i) {
        // Prüfe ob dieser Marker selektiert oder hervorgehoben ist
        if (i == selectedMarkerIndex || i == highlightedMarkerIndex) {
            // Für selektierte/hervorgehobene Marker: verwende updateMarkerColor
            updateMarkerColor(i);
            continue;
        }
        
        const QColor target = colliding[i] ? hitColor : baseColor;
        if (markers[i].color != target) {
            markers[i].color = target;
            markers[i].marker->setColor(target);
        }
    }
}

void SphereWidget::handleCollisions(QVector<bool> &colliding)
{
    if (markers.size() < 2) {
        return;
    }

    constexpr float sphereRadius = 1.0f;
    const float epsilon = 1e-6f;

    for (int i = 0; i < markers.size(); ++i) {
        for (int j = i + 1; j < markers.size(); ++j) {
            auto &a = markers[i];
            auto &b = markers[j];

            const QVector3D pa = a.position.normalized();
            const QVector3D pb = b.position.normalized();

            const float dot = qBound(-1.0f, QVector3D::dotProduct(pa, pb), 1.0f);
            const float angle = qAcos(dot);
            const float minAngle = (a.radius + b.radius) / sphereRadius;

            if (angle > minAngle) {
                continue;
            }

            colliding[i] = true;
            colliding[j] = true;

            QVector3D mid = pa + pb;
            if (mid.lengthSquared() < epsilon) {
                mid = pa;
            }
            mid.normalize();

            QVector3D n = pb - pa;
            n -= QVector3D::dotProduct(n, mid) * mid;
            if (n.lengthSquared() < epsilon) {
                continue;
            }
            n.normalize();

            QVector3D va = a.velocity - QVector3D::dotProduct(a.velocity, mid) * mid;
            QVector3D vb = b.velocity - QVector3D::dotProduct(b.velocity, mid) * mid;

            const float vaN = QVector3D::dotProduct(va, n);
            const float vbN = QVector3D::dotProduct(vb, n);
            const float rel = vaN - vbN;

            if (rel <= 0.0f) {
                continue;
            }

            const QVector3D vaT = va - vaN * n;
            const QVector3D vbT = vb - vbN * n;

            const float m1 = a.radius * a.radius;
            const float m2 = b.radius * b.radius;

            const float newVaN = (vaN * (m1 - m2) + 2.0f * m2 * vbN) / (m1 + m2);
            const float newVbN = (vbN * (m2 - m1) + 2.0f * m1 * vaN) / (m1 + m2);

            a.velocity = vaT + newVaN * n;
            b.velocity = vbT + newVbN * n;

            a.velocity -= QVector3D::dotProduct(a.velocity, pa) * pa;
            b.velocity -= QVector3D::dotProduct(b.velocity, pb) * pb;
        }
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

void SphereWidget::zoomIn()
{
    auto *cam = this->camera();
    QVector3D pos = cam->position();
    pos *= 0.9f;  // 10% näher
    cam->setPosition(pos);
    followMarkerDistance = pos.length();
}

void SphereWidget::zoomOut()
{
    auto *cam = this->camera();
    QVector3D pos = cam->position();
    pos *= 1.1f;  // 10% weiter weg
    cam->setPosition(pos);
    followMarkerDistance = pos.length();
}

QVector<SphereWidget::MarkerInfo> SphereWidget::getMarkersInfo() const
{
    QVector<MarkerInfo> result;
    for (int i = 0; i < markers.size(); ++i) {
        const auto &state = markers[i];
        result.append({
            i,
            state.radius,
            state.density,
            state.color,
            state.position,
            state.velocity
        });
    }
    return result;
}

void SphereWidget::highlightMarker(int markerIndex)
{
    qDebug() << "highlightMarker called with index:" << markerIndex << "total markers:" << markers.size();

    const int previousIndex = highlightedMarkerIndex;
    highlightedMarkerIndex = (markerIndex >= 0 && markerIndex < markers.size()) ? markerIndex : -1;

    if (previousIndex >= 0 && previousIndex < markers.size()) {
        updateMarkerColor(previousIndex);
    }

    if (highlightedMarkerIndex >= 0 && highlightedMarkerIndex < markers.size()) {
        updateMarkerColor(highlightedMarkerIndex);
    }
}

void SphereWidget::clearHighlightedMarker()
{
    highlightMarker(-1);
}

void SphereWidget::setSelectedMarker(int markerIndex)
{
    const int previousIndex = selectedMarkerIndex;
    selectedMarkerIndex = (markerIndex >= 0 && markerIndex < markers.size()) ? markerIndex : -1;

    if (previousIndex >= 0 && previousIndex < markers.size()) {
        updateMarkerColor(previousIndex);
    }

    if (selectedMarkerIndex >= 0 && selectedMarkerIndex < markers.size()) {
        updateMarkerColor(selectedMarkerIndex);
    }
}

void SphereWidget::updateMarkerColor(int markerIndex)
{
    if (markerIndex < 0 || markerIndex >= markers.size()) {
        return;
    }

    QColor colorToApply = markers[markerIndex].color;

    if (selectedMarkerIndex == markerIndex) {
        colorToApply = QColor(0, 255, 0);
    } else if (highlightedMarkerIndex == markerIndex) {
        colorToApply = QColor(255, 0, 0);
    }

    if (markers[markerIndex].marker) {
        markers[markerIndex].marker->setColor(colorToApply);
    }
}

void SphereWidget::setFollowMarker(bool enabled)
{
    followMarkerEnabled = enabled;
    if (enabled) {
        followMarkerDistance = camera()->position().length();
    }
    
    // Deaktiviere den Orbit Controller, wenn Follow aktiv ist
    if (cameraController) {
        cameraController->setEnabled(!enabled);
    }
}

void SphereWidget::setMarkerDensity(int markerIndex, float density)
{
    if (markerIndex < 0 || markerIndex >= markers.size()) {
        return;
    }
    
    markers[markerIndex].density = density;
}

void SphereWidget::setMarkerRadius(int markerIndex, float radius)
{
    if (markerIndex < 0 || markerIndex >= markers.size()) {
        return;
    }
    
    if (radius > 0) {
        markers[markerIndex].radius = radius;
        // Aktualisiere auch die 3D-Geometrie
        if (markers[markerIndex].marker) {
            markers[markerIndex].marker->setMarkerRadius(radius);
        }
    }
}

void SphereWidget::setMarkerVelocityMagnitude(int markerIndex, float magnitude)
{
    if (markerIndex < 0 || markerIndex >= markers.size()) {
        return;
    }
    
    // Behalte die Richtung, aendere nur den Betrag
    QVector3D currentVelocity = markers[markerIndex].velocity;
    float currentMagnitude = currentVelocity.length();
    
    if (currentMagnitude > 0.0001f) {
        // Normalisiere und skaliere mit neuem Betrag
        QVector3D direction = currentVelocity.normalized();
        markers[markerIndex].velocity = direction * magnitude;
    } else {
        // Falls Geschwindigkeit ~0 ist, setze neue Richtung in Z
        markers[markerIndex].velocity = QVector3D(0, 0, magnitude);
    }
}
