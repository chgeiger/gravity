#include "markersettingspanel.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QLocale>

MarkerSettingsPanel::MarkerSettingsPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    auto *markerGroup = new QGroupBox("Marker erzeugen", this);
    auto *markerForm = new QFormLayout(markerGroup);
    markerForm->setLabelAlignment(Qt::AlignLeft);
    markerForm->setFormAlignment(Qt::AlignTop);

    countEdit = new QLineEdit(markerGroup);
    countEdit->setText("8");
    countEdit->setPlaceholderText("z. B. 8");
    countEdit->setValidator(new QIntValidator(0, 10000, countEdit));

    auto *doubleValidator = new QDoubleValidator(0.0, 100.0, 4, this);
    doubleValidator->setLocale(QLocale::c());

    speedMinEdit = new QLineEdit(markerGroup);
    speedMinEdit->setText("0.2");
    speedMinEdit->setPlaceholderText("z. B. 0.2");
    speedMinEdit->setValidator(doubleValidator);

    speedMaxEdit = new QLineEdit(markerGroup);
    speedMaxEdit->setText("0.8");
    speedMaxEdit->setPlaceholderText("z. B. 0.8");
    speedMaxEdit->setValidator(doubleValidator);

    sizeMinEdit = new QLineEdit(markerGroup);
    sizeMinEdit->setText("0.05");
    sizeMinEdit->setPlaceholderText("z. B. 0.05");
    sizeMinEdit->setValidator(doubleValidator);

    sizeMaxEdit = new QLineEdit(markerGroup);
    sizeMaxEdit->setText("0.15");
    sizeMaxEdit->setPlaceholderText("z. B. 0.2");
    sizeMaxEdit->setValidator(doubleValidator);

    markerForm->addRow("Anzahl", countEdit);
    markerForm->addRow("Geschwindigkeit min", speedMinEdit);
    markerForm->addRow("Geschwindigkeit max", speedMaxEdit);
    markerForm->addRow("Größe min", sizeMinEdit);
    markerForm->addRow("Größe max", sizeMaxEdit);

    layout->addWidget(markerGroup);

    generateButton = new QPushButton("Marker erzeugen", this);
    generateButton->setMinimumHeight(32);
    layout->addWidget(generateButton);

    toggleAnimationButton = new QPushButton("Animation stoppen", this);
    toggleAnimationButton->setCheckable(true);
    toggleAnimationButton->setChecked(true);
    toggleAnimationButton->setMinimumHeight(32);
    layout->addWidget(toggleAnimationButton);

    saveButton = new QPushButton("Szenario speichern", this);
    saveButton->setMinimumHeight(32);
    layout->addWidget(saveButton);

    loadButton = new QPushButton("Szenario laden", this);
    loadButton->setMinimumHeight(32);
    layout->addWidget(loadButton);

    clearButton = new QPushButton("Alle Marker löschen", this);
    clearButton->setMinimumHeight(32);
    layout->addWidget(clearButton);

    layout->addStretch(1);

    connect(generateButton, &QPushButton::clicked, this, &MarkerSettingsPanel::emitGenerate);
    connect(toggleAnimationButton, &QPushButton::toggled, this, [this](bool checked) {
        toggleAnimationButton->setText(checked ? "Animation stoppen" : "Animation starten");
        emit animationToggled(checked);
    });
    connect(saveButton, &QPushButton::clicked, this, &MarkerSettingsPanel::saveRequested);
    connect(loadButton, &QPushButton::clicked, this, &MarkerSettingsPanel::loadRequested);
    connect(clearButton, &QPushButton::clicked, this, &MarkerSettingsPanel::clearAllMarkersRequested);
}

void MarkerSettingsPanel::emitGenerate()
{
    bool okCount = false;
    bool okSpeedMin = false;
    bool okSpeedMax = false;
    bool okSizeMin = false;
    bool okSizeMax = false;

    const int count = countEdit->text().toInt(&okCount);
    const float speedMin = speedMinEdit->text().toFloat(&okSpeedMin);
    const float speedMax = speedMaxEdit->text().toFloat(&okSpeedMax);
    const float sizeMin = sizeMinEdit->text().toFloat(&okSizeMin);
    const float sizeMax = sizeMaxEdit->text().toFloat(&okSizeMax);

    if (!okCount || !okSpeedMin || !okSpeedMax || !okSizeMin || !okSizeMax) {
        return;
    }

    emit generateRequested(count, speedMin, speedMax, sizeMin, sizeMax);
}
