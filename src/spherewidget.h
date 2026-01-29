#ifndef SPHEREWIDGET_H
#define SPHEREWIDGET_H

#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>

QT_BEGIN_NAMESPACE
namespace Qt3DCore {
    class QTransform;
}
namespace Qt3DExtras {
    class QSphereMesh;
    class QPhongMaterial;
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

private slots:
    void updateFrame();

private:
    Qt3DCore::QEntity *createScene();
    void createSphere(Qt3DCore::QEntity *rootEntity);
    void createLighting(Qt3DCore::QEntity *rootEntity);

    Qt3DCore::QTransform *sphereTransform;
    float rotationAngle;
};

#endif // SPHEREWIDGET_H
