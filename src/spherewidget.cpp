#include "spherewidget.h"

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPointLight>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <QTimer>
#include <QDebug>

SphereWidget::SphereWidget()
    : Qt3DExtras::Qt3DWindow(), rotationAngle(0.0f), sphereTransform(nullptr)
{
    setTitle("Gravity Simulator - Qt3D");
    
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
    
    qDebug() << "Scene created successfully";
    return rootEntity;
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
    if (sphereTransform) {
        rotationAngle += 0.5f; // Rotate 0.5 degrees per frame
        if (rotationAngle >= 360.0f) {
            rotationAngle -= 360.0f;
        }
        
        auto rotation = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), rotationAngle);
        sphereTransform->setRotation(rotation);
    }
}
