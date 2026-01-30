#include "surface_marker.h"

#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <QVector3D>
#include <QQuaternion>
#include <QtMath>

SurfaceMarker::SurfaceMarker(Qt3DCore::QEntity *parent,
                             float surfaceRadius,
                             float markerRadius,
                             const QColor &color)
    : surfaceRadius(surfaceRadius),
      markerRadius(markerRadius),
      latitudeDeg(0.0f),
      longitudeDeg(0.0f),
      markerEntity(new Qt3DCore::QEntity(parent)),
      transform(new Qt3DCore::QTransform())
{
    auto *mesh = new Qt3DExtras::QCylinderMesh();
    mesh->setRadius(markerRadius);
    mesh->setLength(markerRadius * 0.2f);
    mesh->setRings(16);
    mesh->setSlices(32);

    auto *material = new Qt3DExtras::QPhongMaterial();
    material->setDiffuse(color);
    material->setSpecular(QColor(200, 200, 200));
    material->setShininess(16.0f);

    markerEntity->addComponent(mesh);
    markerEntity->addComponent(material);
    markerEntity->addComponent(transform);

    updateTransform();
}

void SurfaceMarker::setSphericalPosition(float latitudeDeg, float longitudeDeg)
{
    this->latitudeDeg = latitudeDeg;
    this->longitudeDeg = longitudeDeg;
    updateTransform();
}

void SurfaceMarker::updateTransform()
{
    const float latRad = qDegreesToRadians(latitudeDeg);
    const float lonRad = qDegreesToRadians(longitudeDeg);

    const float cosLat = qCos(latRad);
    const QVector3D normal(
        cosLat * qCos(lonRad),
        qSin(latRad),
        cosLat * qSin(lonRad)
    );

    const float offset = markerRadius * 0.1f;
    const QVector3D position = normal * (surfaceRadius + offset);

    const QQuaternion rotation = QQuaternion::rotationTo(QVector3D(0.0f, 1.0f, 0.0f), normal);

    transform->setTranslation(position);
    transform->setRotation(rotation);
}
