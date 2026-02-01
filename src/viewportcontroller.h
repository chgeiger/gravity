#ifndef VIEWPORTCONTROLLER_H
#define VIEWPORTCONTROLLER_H

#include <QWidget>
#include <QColor>

class SphereWidget;

/**
 * @brief ViewportController - Verwaltet den 3D-Viewport und die Sphere-Widget-Initialisierung
 * 
 * Verantwortlichkeiten:
 * - Erstellung und Initialisierung des Qt3D-SphereWidgets
 * - Umwandlung des Qt3DWindow in ein QWidget mittels createWindowContainer
 * - Verwaltung der Viewport-Eigenschaften (z.B. Hintergrundfarbe)
 * - Bereitstellung einer sauberen Schnittstelle fuer den Zugriff auf den 3D-Viewport
 */
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
