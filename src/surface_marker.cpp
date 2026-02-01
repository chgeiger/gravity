#include "surface_marker.h"

#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QRenderPass>
#include <QVector3D>
#include <QQuaternion>
#include <QtMath>
#include <cmath>

namespace {
Qt3DRender::QGeometryRenderer *createSphericalCap(float surfaceRadius, float markerRadius)
{
    auto *geometry = new Qt3DCore::QGeometry();
    auto *renderer = new Qt3DRender::QGeometryRenderer();

    const float minRadius = 0.0001f;
    surfaceRadius = qMax(surfaceRadius, minRadius);
    markerRadius = qMax(markerRadius, minRadius);

    const float maxAngle = static_cast<float>(M_PI) - 0.01f;
    float capAngle = markerRadius / surfaceRadius;
    capAngle = qMin(capAngle, maxAngle);

    const float renderRadius = surfaceRadius + qMax(markerRadius * 0.12f, 0.02f);
    const float thickness = qMax(markerRadius * 0.15f, 0.01f);
    const float innerRadius = qMax(renderRadius - thickness, minRadius);

    const int rings = 16;
    const int slices = 32;
    const int verticesPerRing = slices + 1;
    const int capVertexCount = (rings + 1) * verticesPerRing;
    const int sideVertexCount = 2 * verticesPerRing;
    const int vertexCount = capVertexCount * 2 + sideVertexCount;
    const int capIndexCount = rings * slices * 6;
    const int sideIndexCount = slices * 6;
    const int indexCount = capIndexCount * 2 + sideIndexCount;

    QByteArray vertexBufferData;
    vertexBufferData.resize(vertexCount * 6 * sizeof(float));
    float *v = reinterpret_cast<float *>(vertexBufferData.data());

    // Outer cap
    for (int ring = 0; ring <= rings; ++ring) {
        const float t = static_cast<float>(ring) / static_cast<float>(rings);
        const float theta = capAngle * t;
        const float sinTheta = qSin(theta);
        const float cosTheta = qCos(theta);

        for (int slice = 0; slice <= slices; ++slice) {
            const float s = static_cast<float>(slice) / static_cast<float>(slices);
            const float phi = static_cast<float>(2.0 * M_PI) * s;

            const float sinPhi = qSin(phi);
            const float cosPhi = qCos(phi);

            const QVector3D normal(
                sinTheta * cosPhi,
                cosTheta,
                sinTheta * sinPhi
            );

            const QVector3D position = normal * renderRadius;

            *v++ = position.x();
            *v++ = position.y();
            *v++ = position.z();
            *v++ = normal.x();
            *v++ = normal.y();
            *v++ = normal.z();
        }
    }

    // Inner cap (normals inverted)
    for (int ring = 0; ring <= rings; ++ring) {
        const float t = static_cast<float>(ring) / static_cast<float>(rings);
        const float theta = capAngle * t;
        const float sinTheta = qSin(theta);
        const float cosTheta = qCos(theta);

        for (int slice = 0; slice <= slices; ++slice) {
            const float s = static_cast<float>(slice) / static_cast<float>(slices);
            const float phi = static_cast<float>(2.0 * M_PI) * s;

            const float sinPhi = qSin(phi);
            const float cosPhi = qCos(phi);

            const QVector3D normal(
                sinTheta * cosPhi,
                cosTheta,
                sinTheta * sinPhi
            );

            const QVector3D position = normal * innerRadius;
            const QVector3D inverted = -normal;

            *v++ = position.x();
            *v++ = position.y();
            *v++ = position.z();
            *v++ = inverted.x();
            *v++ = inverted.y();
            *v++ = inverted.z();
        }
    }

    // Side ring
    for (int slice = 0; slice <= slices; ++slice) {
        const float s = static_cast<float>(slice) / static_cast<float>(slices);
        const float phi = static_cast<float>(2.0 * M_PI) * s;

        const float sinPhi = qSin(phi);
        const float cosPhi = qCos(phi);

        const float sinTheta = qSin(capAngle);
        const float cosTheta = qCos(capAngle);

        const QVector3D direction(
            sinTheta * cosPhi,
            cosTheta,
            sinTheta * sinPhi
        );

        const QVector3D normal(
            cosPhi,
            0.0f,
            sinPhi
        );

        const QVector3D outerPos = direction * renderRadius;
        const QVector3D innerPos = direction * innerRadius;

        *v++ = outerPos.x();
        *v++ = outerPos.y();
        *v++ = outerPos.z();
        *v++ = normal.x();
        *v++ = normal.y();
        *v++ = normal.z();

        *v++ = innerPos.x();
        *v++ = innerPos.y();
        *v++ = innerPos.z();
        *v++ = normal.x();
        *v++ = normal.y();
        *v++ = normal.z();
    }

    QByteArray indexBufferData;
    indexBufferData.resize(indexCount * sizeof(quint32));
    quint32 *indices = reinterpret_cast<quint32 *>(indexBufferData.data());

    const int outerStart = 0;
    const int innerStart = capVertexCount;
    const int sideStart = capVertexCount * 2;

    for (int ring = 0; ring < rings; ++ring) {
        for (int slice = 0; slice < slices; ++slice) {
            const int ringStart = ring * verticesPerRing;
            const int nextRingStart = (ring + 1) * verticesPerRing;

            const quint32 a = outerStart + ringStart + slice;
            const quint32 b = outerStart + ringStart + slice + 1;
            const quint32 c = outerStart + nextRingStart + slice;
            const quint32 d = outerStart + nextRingStart + slice + 1;

            *indices++ = a;
            *indices++ = c;
            *indices++ = b;
            *indices++ = b;
            *indices++ = c;
            *indices++ = d;
        }
    }

    for (int ring = 0; ring < rings; ++ring) {
        for (int slice = 0; slice < slices; ++slice) {
            const int ringStart = ring * verticesPerRing;
            const int nextRingStart = (ring + 1) * verticesPerRing;

            const quint32 a = innerStart + ringStart + slice;
            const quint32 b = innerStart + ringStart + slice + 1;
            const quint32 c = innerStart + nextRingStart + slice;
            const quint32 d = innerStart + nextRingStart + slice + 1;

            *indices++ = a;
            *indices++ = b;
            *indices++ = c;
            *indices++ = b;
            *indices++ = d;
            *indices++ = c;
        }
    }

    for (int slice = 0; slice < slices; ++slice) {
        const quint32 outerA = sideStart + slice * 2;
        const quint32 innerA = sideStart + slice * 2 + 1;
        const quint32 outerB = sideStart + (slice + 1) * 2;
        const quint32 innerB = sideStart + (slice + 1) * 2 + 1;

        *indices++ = outerA;
        *indices++ = innerA;
        *indices++ = outerB;
        *indices++ = outerB;
        *indices++ = innerA;
        *indices++ = innerB;
    }

    auto *vertexBuffer = new Qt3DCore::QBuffer(geometry);
    vertexBuffer->setData(vertexBufferData);

    auto *indexBuffer = new Qt3DCore::QBuffer(geometry);
    indexBuffer->setData(indexBufferData);

    auto *positionAttribute = new Qt3DCore::QAttribute();
    positionAttribute->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
    positionAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
    positionAttribute->setVertexSize(3);
    positionAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    positionAttribute->setBuffer(vertexBuffer);
    positionAttribute->setByteStride(6 * sizeof(float));
    positionAttribute->setByteOffset(0);
    positionAttribute->setCount(vertexCount);

    auto *normalAttribute = new Qt3DCore::QAttribute();
    normalAttribute->setName(Qt3DCore::QAttribute::defaultNormalAttributeName());
    normalAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
    normalAttribute->setVertexSize(3);
    normalAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    normalAttribute->setBuffer(vertexBuffer);
    normalAttribute->setByteStride(6 * sizeof(float));
    normalAttribute->setByteOffset(3 * sizeof(float));
    normalAttribute->setCount(vertexCount);

    auto *indexAttribute = new Qt3DCore::QAttribute();
    indexAttribute->setAttributeType(Qt3DCore::QAttribute::IndexAttribute);
    indexAttribute->setVertexBaseType(Qt3DCore::QAttribute::UnsignedInt);
    indexAttribute->setBuffer(indexBuffer);
    indexAttribute->setCount(indexCount);

    geometry->addAttribute(positionAttribute);
    geometry->addAttribute(normalAttribute);
    geometry->addAttribute(indexAttribute);

    renderer->setGeometry(geometry);
    renderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);

    return renderer;
}
}

SurfaceMarker::SurfaceMarker(Qt3DCore::QEntity *parent,
                             float surfaceRadius,
                             float markerRadius,
                             const QColor &color)
    : surfaceRadius(surfaceRadius),
      markerRadius(markerRadius),
      latitudeDeg(0.0f),
      longitudeDeg(0.0f),
      markerEntity(new Qt3DCore::QEntity(parent)),
            transform(new Qt3DCore::QTransform()),
            geometryRenderer(createSphericalCap(surfaceRadius, markerRadius)),
            material(new Qt3DExtras::QPhongMaterial())
{
        material->setDiffuse(color);
        material->setAmbient(color);
        material->setSpecular(QColor(200, 200, 200));
        material->setShininess(16.0f);

    if (auto *effect = material->effect()) {
        const auto techniques = effect->techniques();
        for (auto *technique : techniques) {
            const auto renderPasses = technique->renderPasses();
            for (auto *pass : renderPasses) {
                auto *cullFace = new Qt3DRender::QCullFace(pass);
                cullFace->setMode(Qt3DRender::QCullFace::NoCulling);
                pass->addRenderState(cullFace);
            }
        }
    }

        markerEntity->addComponent(geometryRenderer);
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

void SurfaceMarker::setColor(const QColor &color)
{
    if (!material) {
        return;
    }
    material->setDiffuse(color);
    material->setAmbient(color);
}

void SurfaceMarker::setMarkerRadius(float radius)
{
    if (radius <= 0.0f) {
        return;
    }
    
    markerRadius = radius;
    
    // Erstelle die Geometrie neu mit dem neuen Radius
    if (geometryRenderer) {
        delete geometryRenderer;
    }
    
    geometryRenderer = createSphericalCap(surfaceRadius, markerRadius);
    if (markerEntity && geometryRenderer) {
        markerEntity->addComponent(geometryRenderer);
    }
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

    const QQuaternion rotation = QQuaternion::rotationTo(QVector3D(0.0f, 1.0f, 0.0f), normal);

    transform->setRotation(rotation);
    transform->setScale(1.0f);
    transform->setTranslation(QVector3D(0.0f, 0.0f, 0.0f));
}
