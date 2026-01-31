#ifndef SPHEREWIDGET_H
#define SPHEREWIDGET_H

#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QForwardRenderer>
#include <QColor>
#include <QVector>
#include <QElapsedTimer>
#include <QVector3D>
#include <QJsonObject>
#include <QColor>

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

    void generateMarkers(int count, float speed, float size);
    void setAnimationEnabled(bool enabled);
    bool isAnimationEnabled() const { return animationEnabled; }
    void clearMarkers();
    void zoomIn();
    void zoomOut();
    void highlightMarker(int markerIndex);
    void setSelectedMarker(int markerIndex);
    void clearHighlightedMarker();
    void setFollowMarker(bool enabled);
    
    struct MarkerInfo {
        int index;
        float radius;
        QColor color;
        QVector3D position;
        QVector3D velocity;
    };
    QVector<MarkerInfo> getMarkersInfo() const;

    QJsonObject exportScenario() const;
    bool applyScenario(const QJsonObject &scenario);
    
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
    void handleCollisions(QVector<bool> &colliding);
    void updateMarkerColor(int markerIndex);

    Qt3DCore::QTransform *sphereTransform;
    Qt3DExtras::QOrbitCameraController *cameraController;
    Qt3DCore::QEntity *rootEntity;
    struct MarkerState {
        SurfaceMarker *marker;
        QVector3D position; // unit vector on sphere
        QVector3D velocity; // tangent vector (units: sphere radii per second)
        float radius;
        QColor color;
    };
    QVector<MarkerState> markers;
    QElapsedTimer frameTimer;
    qint64 lastFrameMs;
    QTimer *animationTimer;
    bool animationEnabled;
    int highlightedMarkerIndex;
    int selectedMarkerIndex;
    bool followMarkerEnabled;
        float followMarkerDistance; // Distance for following the marker
};

#endif // SPHEREWIDGET_H
