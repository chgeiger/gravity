#include "spherewidget.h"
#include <QTimer>
#include <cmath>
#include <vector>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>

QOpenGLFunctions_3_3_Core* glFunc = nullptr;

SphereWidget::SphereWidget(QWidget *parent)
    : QOpenGLWidget(parent), rotation(0.0f), VAO(0), VBO(0), EBO(0), vertexCount(0) {
    setFocusPolicy(Qt::StrongFocus);
    
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SphereWidget::updateRotation);
    timer->start(16); // ~60 FPS
}

SphereWidget::~SphereWidget() {
    makeCurrent();
    if (glFunc && VAO != 0) glFunc->glDeleteVertexArrays(1, &VAO);
    if (glFunc && VBO != 0) glFunc->glDeleteBuffers(1, &VBO);
    if (glFunc && EBO != 0) glFunc->glDeleteBuffers(1, &EBO);
    doneCurrent();
}

void SphereWidget::initializeGL() {
    glFunc = new QOpenGLFunctions_3_3_Core();
    if (!glFunc->initializeOpenGLFunctions()) {
        return;
    }
    
    glFunc->glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glFunc->glEnable(GL_DEPTH_TEST);

    createSphere();
}

void SphereWidget::resizeGL(int w, int h) {
    if (glFunc) {
        glFunc->glViewport(0, 0, w, h);
        projection.setToIdentity();
        projection.perspective(45.0f, (float)w / (float)h, 0.1f, 100.0f);
    }
}

void SphereWidget::paintGL() {
    if (!glFunc) return;
    
    glFunc->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    view.setToIdentity();
    view.translate(0, 0, -3);

    model.setToIdentity();
    model.rotate(rotation, 1, 1, 0);

    glFunc->glBindVertexArray(VAO);
    glFunc->glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
}

void SphereWidget::updateRotation() {
    rotation += 1.0f;
    update();
}

void SphereWidget::createSphere() {
    if (!glFunc) return;
    
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;

    const int latSegments = 30;
    const int lonSegments = 30;
    const float radius = 1.0f;

    for (int i = 0; i <= latSegments; ++i) {
        float lat0 = M_PI * (-0.5f + (float)i / latSegments);
        float lat1 = M_PI * (-0.5f + (float)(i + 1) / latSegments);

        float sinLat0 = sin(lat0);
        float cosLat0 = cos(lat0);

        for (int j = 0; j <= lonSegments; ++j) {
            float lon = 2 * M_PI * j / lonSegments;
            float sinLon = sin(lon);
            float cosLon = cos(lon);

            float x = cosLat0 * cosLon * radius;
            float y = sinLat0 * radius;
            float z = cosLat0 * sinLon * radius;

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(0.5f + 0.5f * x);
            vertices.push_back(0.5f + 0.5f * y);
            vertices.push_back(0.5f + 0.5f * z);
        }
    }

    for (int i = 0; i < latSegments; ++i) {
        int a = i * (lonSegments + 1);
        int b = a + lonSegments + 1;

        for (int j = 0; j < lonSegments; ++j) {
            indices.push_back(a + j);
            indices.push_back(b + j);
            indices.push_back(a + j + 1);

            indices.push_back(b + j);
            indices.push_back(b + j + 1);
            indices.push_back(a + j + 1);
        }
    }

    vertexCount = indices.size();

    glFunc->glGenVertexArrays(1, &VAO);
    glFunc->glGenBuffers(1, &VBO);
    glFunc->glGenBuffers(1, &EBO);

    glFunc->glBindVertexArray(VAO);
    glFunc->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glFunc->glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glFunc->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glFunc->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glFunc->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glFunc->glEnableVertexAttribArray(0);

    glFunc->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glFunc->glEnableVertexAttribArray(1);
}
