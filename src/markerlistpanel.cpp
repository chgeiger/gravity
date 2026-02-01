#include "markerlistpanel.h"
#include "spherewidget.h"

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
        radiusItem->setText(0, QString("Radius: %1").arg(markerInfo.radius, 0, 'f', 3));

        auto *colorItem = new QTreeWidgetItem(markerItem);
        colorItem->setText(0, QString("Farbe: RGB(%1, %2, %3)")
            .arg(markerInfo.color.red())
            .arg(markerInfo.color.green())
            .arg(markerInfo.color.blue()));

        auto *densityItem = new QTreeWidgetItem(markerItem);
        densityItem->setText(0, QString("Dichte: %1").arg(markerInfo.density, 0, 'f', 3));

        auto *posItem = new QTreeWidgetItem(markerItem);
        posItem->setText(0, QString("Position: (%1, %2, %3)")
            .arg(markerInfo.position.x(), 0, 'f', 3)
            .arg(markerInfo.position.y(), 0, 'f', 3)
            .arg(markerInfo.position.z(), 0, 'f', 3));

        auto *velItem = new QTreeWidgetItem(markerItem);
        velItem->setText(0, QString("Geschwindigkeit: (%1, %2, %3)")
            .arg(markerInfo.velocity.x(), 0, 'f', 3)
            .arg(markerInfo.velocity.y(), 0, 'f', 3)
            .arg(markerInfo.velocity.z(), 0, 'f', 3));

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
