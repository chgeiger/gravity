#include "spherewidget.h"
#include <QOpenGLFunctions_3_3_Core>
#include <QTimer>
#include <cmath>
#include <vector>
#include <QDebug>

QOpenGLFunctions_3_3_Core* glFunc = nullptr;

// GLSL Vertex Shader
const char *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vs_out.FragPos = vec3(model * vec4(position, 1.0));
    vs_out.Normal = mat3(transpose(inverse(model))) * normal;
    gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
}
)";

// GLSL Fragment Shader
const char *fragmentShaderSource = R"(
#version 330 core
in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
} fs_in;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectColor;

void main()
{
    // Ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * objectColor;

    // Diffuse
    vec3 norm = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * objectColor;

    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * vec3(1.0);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
)";

SphereWidget::SphereWidget(QWidget *parent)
    : QOpenGLWidget(parent), rotation(0.0f), VAO(0), VBO(0), EBO(0), 
      vertexCount(0), shaderProgram(nullptr) {
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
    if (shaderProgram) delete shaderProgram;
    doneCurrent();
}

void SphereWidget::createShaderProgram() {
    shaderProgram = new QOpenGLShaderProgram();
    
    if (!shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource)) {
        qDebug() << "Vertex shader compilation failed:" << shaderProgram->log();
    }
    
    if (!shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource)) {
        qDebug() << "Fragment shader compilation failed:" << shaderProgram->log();
    }
    
    if (!shaderProgram->link()) {
        qDebug() << "Shader program linking failed:" << shaderProgram->log();
    }
}

void SphereWidget::initializeGL() {
    glFunc = new QOpenGLFunctions_3_3_Core();
    if (!glFunc->initializeOpenGLFunctions()) {
        return;
    }
    
    // Blauer Hintergrund
    glFunc->glClearColor(0.1f, 0.3f, 0.8f, 1.0f);
    glFunc->glEnable(GL_DEPTH_TEST);

    createShaderProgram();
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
    if (!glFunc || !shaderProgram) return;
    
    glFunc->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaderProgram->bind();

    // Setup matrices
    view.setToIdentity();
    view.translate(0, 0, -3);

    model.setToIdentity();
    model.rotate(rotation, 0, 1, 0);

    // Pass matrices to shader
    shaderProgram->setUniformValue("model", model);
    shaderProgram->setUniformValue("view", view);
    shaderProgram->setUniformValue("projection", projection);

    // Setup lighting
    QVector3D lightPos(2.0f, 2.0f, 2.0f); // Seitliche Beleuchtung
    QVector3D viewPos(0.0f, 0.0f, 3.0f);
    QVector3D objectColor(1.0f, 0.4f, 0.2f); // Orange-rötliche Farbe

    shaderProgram->setUniformValue("lightPos", lightPos);
    shaderProgram->setUniformValue("viewPos", viewPos);
    shaderProgram->setUniformValue("objectColor", objectColor);

    // Draw sphere
    glFunc->glBindVertexArray(VAO);
    glFunc->glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);

    shaderProgram->release();
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

    // Generiere Vertex-Positionen und Normalen
    for (int i = 0; i <= latSegments; ++i) {
        float lat0 = M_PI * (-0.5f + (float)i / latSegments);
        float lat1 = M_PI * (-0.5f + (float)(i + 1) / latSegments);

        float sinLat0 = sin(lat0);
        float cosLat0 = cos(lat0);

        for (int j = 0; j <= lonSegments; ++j) {
            float lon = 2 * M_PI * j / lonSegments;
            float sinLon = sin(lon);
            float cosLon = cos(lon);

            // Position
            float x = cosLat0 * cosLon * radius;
            float y = sinLat0 * radius;
            float z = cosLat0 * sinLon * radius;

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normal (für eine Sphäre = Position normalisiert)
            vertices.push_back(cosLat0 * cosLon);
            vertices.push_back(sinLat0);
            vertices.push_back(cosLat0 * sinLon);
        }
    }

    // Generiere Indices
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

    // Position attribute
    glFunc->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glFunc->glEnableVertexAttribArray(0);

    // Normal attribute
    glFunc->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glFunc->glEnableVertexAttribArray(1);
}
