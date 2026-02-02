#include "markerlistpanel.h"
#include "spherewidget.h"
#include "editablepropertywidget.h"

#include <QListWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QDebug>

MarkerListPanel::MarkerListPanel(SphereWidget *sphereWidget, QWidget *parent)
    : QWidget(parent), sphereWidget(sphereWidget), currentlySelectedMarkerIndex(-1)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    markersListWidget = new QListWidget(this);
    layout->addWidget(markersListWidget);

    // Gruppe für ausgewählten Marker
    selectedMarkerGroup = new QGroupBox("Ausgewählter Marker", this);
    selectedMarkerGroup->setVisible(false);
    auto *selectedMarkerLayout = new QVBoxLayout(selectedMarkerGroup);
    selectedMarkerLayout->setContentsMargins(8, 8, 8, 8);
    selectedMarkerLayout->setSpacing(8);

    selectedRadiusWidget = new EditablePropertyWidget("Radius:", "0.0");
    selectedMarkerLayout->addWidget(selectedRadiusWidget);

    selectedDensityWidget = new EditablePropertyWidget("Dichte:", "0.0");
    selectedMarkerLayout->addWidget(selectedDensityWidget);

    selectedVelocityWidget = new EditablePropertyWidget("Geschwindigkeit:", "0.0");
    selectedMarkerLayout->addWidget(selectedVelocityWidget);

    layout->addWidget(selectedMarkerGroup);

    // Verbinde Änderungen mit der Simulation
    connect(selectedRadiusWidget, &EditablePropertyWidget::valueChanged, this,
            [this](const QString &newValue) {
                if (currentlySelectedMarkerIndex >= 0) {
                    bool ok;
                    float radius = newValue.toFloat(&ok);
                    if (ok && radius > 0) {
                        this->sphereWidget->setMarkerRadius(currentlySelectedMarkerIndex, radius);
                    }
                }
            });

    connect(selectedDensityWidget, &EditablePropertyWidget::valueChanged, this,
            [this](const QString &newValue) {
                if (currentlySelectedMarkerIndex >= 0) {
                    bool ok;
                    float density = newValue.toFloat(&ok);
                    if (ok && density > 0) {
                        this->sphereWidget->setMarkerDensity(currentlySelectedMarkerIndex, density);
                    }
                }
            });

    connect(selectedVelocityWidget, &EditablePropertyWidget::valueChanged, this,
            [this](const QString &newValue) {
                if (currentlySelectedMarkerIndex >= 0) {
                    bool ok;
                    float magnitude = newValue.toFloat(&ok);
                    if (ok && magnitude >= 0) {
                        this->sphereWidget->setMarkerVelocityMagnitude(currentlySelectedMarkerIndex, magnitude);
                    }
                }
            });

    auto *markerControlsGroup = new QGroupBox("Marker folgen", this);
    auto *markerControlsForm = new QFormLayout(markerControlsGroup);
    markerControlsForm->setLabelAlignment(Qt::AlignLeft);
    markerControlsForm->setFormAlignment(Qt::AlignTop);

    markerActionCheckBox = new QCheckBox(markerControlsGroup);
    markerActionCheckBox->setChecked(false);

    markerSelectionCombo = new QComboBox(markerControlsGroup);
    markerSelectionCombo->addItem("Keine Marker vorhanden");

    markerControlsForm->addRow("Aktiv", markerActionCheckBox);
    markerControlsForm->addRow("Marker wählen", markerSelectionCombo);

    layout->addWidget(markerControlsGroup);

    // Baumansicht soll den verfügbaren Platz einnehmen
    layout->setStretch(0, 1);
    layout->setStretch(1, 0);
    layout->setStretch(2, 0);

    connect(markerSelectionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                this->sphereWidget->setSelectedMarker(index);
            });

    connect(markerActionCheckBox, &QCheckBox::toggled, this,
            [this](bool checked) {
                this->sphereWidget->setFollowMarker(checked);
            });

    connect(markersListWidget, &QListWidget::itemClicked, this, &MarkerListPanel::onMarkerSelectionChanged);
}

void MarkerListPanel::refreshMarkersTree()
{
    markersListWidget->clear();
    const int previousComboIndex = markerSelectionCombo ? markerSelectionCombo->currentIndex() : -1;
    if (markerSelectionCombo) {
        markerSelectionCombo->clear();
    }

    const auto markers = sphereWidget->getMarkersInfo();

    if (markerSelectionCombo) {
        if (markers.isEmpty()) {
            markerSelectionCombo->addItem("Keine Marker vorhanden");
            markerSelectionCombo->setEnabled(false);
        } else {
            markerSelectionCombo->setEnabled(true);
        }
    }

    for (const auto &markerInfo : markers) {
        markersListWidget->addItem(QString("Marker %1").arg(markerInfo.index + 1));

        if (markerSelectionCombo) {
            markerSelectionCombo->addItem(QString("Marker %1").arg(markerInfo.index + 1));
        }
    }

    if (markerSelectionCombo) {
        if (markers.isEmpty()) {
            markerSelectionCombo->setCurrentIndex(0);
            sphereWidget->setSelectedMarker(-1);
        } else {
            const int maxIndex = markers.size() - 1;
            const int nextIndex = qBound(0, previousComboIndex, maxIndex);
            markerSelectionCombo->setCurrentIndex(nextIndex);
            sphereWidget->setSelectedMarker(nextIndex);
        }
    }
}

void MarkerListPanel::onMarkerSelectionChanged()
{
    qDebug() << "onMarkerSelectionChanged called";

    auto *selectedItem = markersListWidget->currentItem();

    if (!selectedItem) {
        qDebug() << "No item selected";
        sphereWidget->clearHighlightedMarker();
        currentlySelectedMarkerIndex = -1;
        selectedMarkerGroup->setVisible(false);
        return;
    }

    // Finde den Index des Markers in der Liste
    int markerIndex = markersListWidget->row(selectedItem);
    qDebug() << "Marker index:" << markerIndex;

    if (markerIndex >= 0) {
        sphereWidget->highlightMarker(markerIndex);
        currentlySelectedMarkerIndex = markerIndex;
        updateSelectedMarkerProperties();
    }
}

void MarkerListPanel::updateSelectedMarkerProperties()
{
    if (currentlySelectedMarkerIndex < 0) {
        selectedMarkerGroup->setVisible(false);
        return;
    }

    const auto markers = sphereWidget->getMarkersInfo();
    if (currentlySelectedMarkerIndex >= markers.size()) {
        selectedMarkerGroup->setVisible(false);
        return;
    }

    const auto &markerInfo = markers[currentlySelectedMarkerIndex];
    
    selectedMarkerGroup->setTitle(QString("Marker %1").arg(markerInfo.index + 1));
    selectedRadiusWidget->setValue(QString::number(markerInfo.radius, 'f', 3));
    selectedDensityWidget->setValue(QString::number(markerInfo.density, 'f', 3));
    
    float velocityMagnitude = markerInfo.velocity.length();
    selectedVelocityWidget->setValue(QString::number(velocityMagnitude, 'f', 3));
    
    selectedMarkerGroup->setVisible(true);
}
