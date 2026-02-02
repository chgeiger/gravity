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
    : QWidget(parent), sphereWidget(sphereWidget)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    markersListWidget = new QListWidget(this);
    markersListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
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
                bool ok;
                float radius = newValue.toFloat(&ok);
                if (ok && radius > 0) {
                    for (int markerIndex : selectedMarkerIndices) {
                        this->sphereWidget->setMarkerRadius(markerIndex, radius);
                    }
                }
            });

    connect(selectedDensityWidget, &EditablePropertyWidget::valueChanged, this,
            [this](const QString &newValue) {
                bool ok;
                float density = newValue.toFloat(&ok);
                if (ok && density > 0) {
                    for (int markerIndex : selectedMarkerIndices) {
                        this->sphereWidget->setMarkerDensity(markerIndex, density);
                    }
                }
            });

    connect(selectedVelocityWidget, &EditablePropertyWidget::valueChanged, this,
            [this](const QString &newValue) {
                bool ok;
                float magnitude = newValue.toFloat(&ok);
                if (ok && magnitude >= 0) {
                    for (int markerIndex : selectedMarkerIndices) {
                        this->sphereWidget->setMarkerVelocityMagnitude(markerIndex, magnitude);
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

    connect(markersListWidget, &QListWidget::itemSelectionChanged, this, &MarkerListPanel::onMarkerSelectionChanged);
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

    auto selectedItems = markersListWidget->selectedItems();

    if (selectedItems.isEmpty()) {
        qDebug() << "No items selected";
        sphereWidget->clearHighlightedMarker();
        selectedMarkerIndices.clear();
        selectedMarkerGroup->setVisible(false);
        return;
    }

    // Sammle alle ausgewählten Marker-Indizes
    selectedMarkerIndices.clear();
    for (auto *item : selectedItems) {
        int markerIndex = markersListWidget->row(item);
        if (markerIndex >= 0) {
            selectedMarkerIndices.append(markerIndex);
        }
    }

    qDebug() << "Selected marker indices:" << selectedMarkerIndices;

    // Highlighte den ersten ausgewählten Marker
    if (!selectedMarkerIndices.isEmpty()) {
        sphereWidget->highlightMarker(selectedMarkerIndices.first());
        updateSelectedMarkerProperties();
    }
}

void MarkerListPanel::updateSelectedMarkerProperties()
{
    if (selectedMarkerIndices.isEmpty()) {
        selectedMarkerGroup->setVisible(false);
        return;
    }

    const auto markers = sphereWidget->getMarkersInfo();
    
    // Prüfe ob alle Indizes gültig sind
    for (int index : selectedMarkerIndices) {
        if (index >= markers.size()) {
            selectedMarkerGroup->setVisible(false);
            return;
        }
    }

    // Titel setzen
    if (selectedMarkerIndices.size() == 1) {
        selectedMarkerGroup->setTitle(QString("Marker %1").arg(selectedMarkerIndices.first() + 1));
    } else {
        selectedMarkerGroup->setTitle(QString("%1 Marker ausgewählt").arg(selectedMarkerIndices.size()));
    }

    // Prüfe ob alle ausgewählten Marker den gleichen Radius haben
    bool sameRadius = true;
    float firstRadius = markers[selectedMarkerIndices.first()].radius;
    for (int i = 1; i < selectedMarkerIndices.size(); ++i) {
        if (qAbs(markers[selectedMarkerIndices[i]].radius - firstRadius) > 0.0001f) {
            sameRadius = false;
            break;
        }
    }
    
    if (sameRadius) {
        selectedRadiusWidget->setValue(QString::number(firstRadius, 'f', 3));
    } else {
        selectedRadiusWidget->setValue("");
    }

    // Prüfe ob alle ausgewählten Marker die gleiche Dichte haben
    bool sameDensity = true;
    float firstDensity = markers[selectedMarkerIndices.first()].density;
    for (int i = 1; i < selectedMarkerIndices.size(); ++i) {
        if (qAbs(markers[selectedMarkerIndices[i]].density - firstDensity) > 0.0001f) {
            sameDensity = false;
            break;
        }
    }
    
    if (sameDensity) {
        selectedDensityWidget->setValue(QString::number(firstDensity, 'f', 3));
    } else {
        selectedDensityWidget->setValue("");
    }

    // Prüfe ob alle ausgewählten Marker die gleiche Geschwindigkeit haben
    bool sameVelocity = true;
    float firstVelocityMagnitude = markers[selectedMarkerIndices.first()].velocity.length();
    for (int i = 1; i < selectedMarkerIndices.size(); ++i) {
        float velocityMagnitude = markers[selectedMarkerIndices[i]].velocity.length();
        if (qAbs(velocityMagnitude - firstVelocityMagnitude) > 0.0001f) {
            sameVelocity = false;
            break;
        }
    }
    
    if (sameVelocity) {
        selectedVelocityWidget->setValue(QString::number(firstVelocityMagnitude, 'f', 3));
    } else {
        selectedVelocityWidget->setValue("");
    }
    
    selectedMarkerGroup->setVisible(true);
}
