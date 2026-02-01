#include "viewportcontroller.h"
#include "spherewidget.h"

#include <QWidget>

ViewportController::ViewportController()
    : sphereWidget(nullptr), containerWidget(nullptr)
{
    // Create Qt3D sphere widget (NOT as parent!)
    sphereWidget = new SphereWidget();

    // Convert Qt3DWindow to QWidget using createWindowContainer
    containerWidget = QWidget::createWindowContainer(sphereWidget);
    containerWidget->setMinimumSize(800, 600);
}

ViewportController::~ViewportController()
{
    if (containerWidget) {
        delete containerWidget;
        // sphereWidget is deleted by containerWidget
    }
}

void ViewportController::setBackgroundColor(const QColor &color)
{
    if (sphereWidget) {
        sphereWidget->setBackgroundColor(color);
    }
}
