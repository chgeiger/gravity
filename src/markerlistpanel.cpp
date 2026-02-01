#include "markerlistpanel.h"
#include "spherewidget.h"
#include "editablepropertywidget.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
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

    markersTreeWidget = new QTreeWidget(this);
    markersTreeWidget->setHeaderLabel("Marker");
    markersTreeWidget->setColumnCount(1);
    layout->addWidget(markersTreeWidget);

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

    connect(markerSelectionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                this->sphereWidget->setSelectedMarker(index);
            });

    connect(markerActionCheckBox, &QCheckBox::toggled, this,
            [this](bool checked) {
                this->sphereWidget->setFollowMarker(checked);
            });

    connect(markersTreeWidget, &QTreeWidget::itemClicked, this, &MarkerListPanel::onMarkerSelectionChanged);
}

void MarkerListPanel::refreshMarkersTree()
{
    markersTreeWidget->clear();
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
        auto *markerItem = new QTreeWidgetItem();
        markerItem->setText(0, QString("Marker %1").arg(markerInfo.index + 1));

        auto *radiusItem = new QTreeWidgetItem(markerItem);
        auto *radiusWidget = new EditablePropertyWidget("Radius:", 
                                                        QString::number(markerInfo.radius, 'f', 3));
        markersTreeWidget->setItemWidget(radiusItem, 0, radiusWidget);
        
        // Verbinde Aenderungen mit der Simulation
        connect(radiusWidget, &EditablePropertyWidget::valueChanged, this,
                [this, markerIndex = markerInfo.index](const QString &newValue) {
                    bool ok;
                    float radius = newValue.toFloat(&ok);
                    if (ok && radius > 0) {
                        sphereWidget->setMarkerRadius(markerIndex, radius);
                    }
                });

        auto *densityItem = new QTreeWidgetItem(markerItem);
        auto *densityWidget = new EditablePropertyWidget("Dichte:", 
                                                         QString::number(markerInfo.density, 'f', 3));
        markersTreeWidget->setItemWidget(densityItem, 0, densityWidget);
        
        // Verbinde Aenderungen mit der Simulation
        connect(densityWidget, &EditablePropertyWidget::valueChanged, this,
                [this, markerIndex = markerInfo.index](const QString &newValue) {
                    bool ok;
                    float density = newValue.toFloat(&ok);
                    if (ok && density > 0) {
                        sphereWidget->setMarkerDensity(markerIndex, density);
                    }
                });

        auto *velItem = new QTreeWidgetItem(markerItem);
        float velocityMagnitude = markerInfo.velocity.length();
        auto *velocityWidget = new EditablePropertyWidget("Geschwindigkeit:", 
                                                          QString::number(velocityMagnitude, 'f', 3));
        markersTreeWidget->setItemWidget(velItem, 0, velocityWidget);
        
        // Verbinde Aenderungen mit der Simulation
        connect(velocityWidget, &EditablePropertyWidget::valueChanged, this,
                [this, markerIndex = markerInfo.index](const QString &newValue) {
                    bool ok;
                    float magnitude = newValue.toFloat(&ok);
                    if (ok && magnitude >= 0) {
                        sphereWidget->setMarkerVelocityMagnitude(markerIndex, magnitude);
                    }
                });

        markersTreeWidget->addTopLevelItem(markerItem);
        markerItem->setExpanded(false);

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

    QTreeWidgetItem *selectedItem = markersTreeWidget->currentItem();

    if (!selectedItem) {
        qDebug() << "No item selected";
        sphereWidget->clearHighlightedMarker();
        return;
    }

    qDebug() << "Selected item:" << selectedItem->text(0);

    // Finde den Parent-Item (Top-Level Item)
    QTreeWidgetItem *markerItem = selectedItem;
    while (markerItem->parent() != nullptr) {
        markerItem = markerItem->parent();
    }

    qDebug() << "Marker item:" << markerItem->text(0);

    // Finde den Index des Markers
    int markerIndex = markersTreeWidget->indexOfTopLevelItem(markerItem);
    qDebug() << "Marker index:" << markerIndex;

    if (markerIndex >= 0) {
        sphereWidget->highlightMarker(markerIndex);
    }
}
