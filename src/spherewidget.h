#ifndef SPHEREWIDGET_H
#define SPHEREWIDGET_H

#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QForwardRenderer>
#include <QColor>

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

    Qt3DCore::QTransform *sphereTransform;
    float rotationAngle;
    Qt3DExtras::QOrbitCameraController *cameraController;
};

#endif // SPHEREWIDGET_H
