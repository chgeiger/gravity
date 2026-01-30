#ifndef SURFACE_MARKER_H
#define SURFACE_MARKER_H

#include <Qt3DCore/QEntity>
#include <QColor>

QT_BEGIN_NAMESPACE
namespace Qt3DCore {
    class QTransform;
}
QT_END_NAMESPACE

class SurfaceMarker {
public:
    SurfaceMarker(Qt3DCore::QEntity *parent,
                  float surfaceRadius,
                  float markerRadius,
                  const QColor &color);
    ~SurfaceMarker() = default;

    void setSphericalPosition(float latitudeDeg, float longitudeDeg);

    Qt3DCore::QEntity *entity() const { return markerEntity; }

private:
    void updateTransform();

    float surfaceRadius;
    float markerRadius;
    float latitudeDeg;
    float longitudeDeg;

    Qt3DCore::QEntity *markerEntity;
    Qt3DCore::QTransform *transform;
};

#endif // SURFACE_MARKER_H
