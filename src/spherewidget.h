#ifndef SPHEREWIDGET_H
#define SPHEREWIDGET_H

#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QForwardRenderer>
#include <QColor>
#include <QVector>
#include <QElapsedTimer>
#include <QVector3D>

#include "surface_marker.h"

QT_BEGIN_NAMESPACE
namespace Qt3DCore {
    class QTransform;
}
namespace Qt3DExtras {
    class QSphereMesh;
    class QPhongMaterial;
    class QOrbitCameraController;
}
namespace Qt3DRender {
    class QPointLight;
}
QT_END_NAMESPACE

class SphereWidget : public Qt3DExtras::Qt3DWindow {
    Q_OBJECT

public:
    SphereWidget();
    ~SphereWidget() = default;

    void generateMarkers(int count, float speedMin, float speedMax, float sizeMin, float sizeMax);
    
    inline void setBackgroundColor(const QColor &color) {
        auto fg = defaultFrameGraph();
        if (fg) {
            fg->setClearColor(color);
        }
    }

private slots:
    void updateFrame();

private:
    Qt3DCore::QEntity *createScene();
    void createSphere(Qt3DCore::QEntity *rootEntity);
    void createLighting(Qt3DCore::QEntity *rootEntity);
    void createMarkers(Qt3DCore::QEntity *rootEntity);
    void updateMarkers(float deltaSeconds);
    void clearMarkers();

    Qt3DCore::QTransform *sphereTransform;
    float rotationAngle;
    Qt3DExtras::QOrbitCameraController *cameraController;
    Qt3DCore::QEntity *rootEntity;
    struct MarkerState {
        SurfaceMarker *marker;
        QVector3D position; // unit vector on sphere
        QVector3D velocity; // tangent vector (units: sphere radii per second)
    };
    QVector<MarkerState> markers;
    QElapsedTimer frameTimer;
    qint64 lastFrameMs;
};

#endif // SPHEREWIDGET_H
