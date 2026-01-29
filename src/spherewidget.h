#ifndef SPHEREWIDGET_H
#define SPHEREWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QVector3D>
#include <QTimer>

class SphereWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    SphereWidget(QWidget *parent = nullptr);
    ~SphereWidget();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private slots:
    void updateRotation();

private:
    void createSphere();
    void drawSphere();

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;
    float rotation;
    GLuint VAO, VBO, EBO;
    GLuint vertexCount;
    QTimer *timer;
};

#endif // SPHEREWIDGET_H
