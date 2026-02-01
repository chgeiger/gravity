#ifndef VIEWPORTCONTROLLER_H
#define VIEWPORTCONTROLLER_H

#include <QWidget>
#include <QColor>

class SphereWidget;

class ViewportController {
public:
    explicit ViewportController();
    ~ViewportController();

    QWidget *getContainerWidget() const { return containerWidget; }
    SphereWidget *getSphereWidget() const { return sphereWidget; }

    void setBackgroundColor(const QColor &color);

private:
    SphereWidget *sphereWidget;
    QWidget *containerWidget;
};

#endif // VIEWPORTCONTROLLER_H
