#include "markersettingspanel.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QLocale>
#include <QSlider>
#include <QLabel>

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

    speedEdit = new QLineEdit(markerGroup);
    speedEdit->setText("0.5");
    speedEdit->setPlaceholderText("z. B. 0.5");
    speedEdit->setValidator(doubleValidator);

    sizeEdit = new QLineEdit(markerGroup);
    sizeEdit->setText("0.1");
    sizeEdit->setPlaceholderText("z. B. 0.1");
    sizeEdit->setValidator(doubleValidator);

    densityEdit = new QLineEdit(markerGroup);
    densityEdit->setText("1.0");
    densityEdit->setPlaceholderText("z. B. 1.0");
    densityEdit->setValidator(doubleValidator);

    markerForm->addRow("Anzahl", countEdit);
    markerForm->addRow("Geschwindigkeit", speedEdit);
    markerForm->addRow("Größe", sizeEdit);
    markerForm->addRow("Dichte", densityEdit);

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

    auto *zoomLayout = new QHBoxLayout();
    zoomOutButton = new QPushButton("-", this);
    zoomOutButton->setMinimumHeight(32);
    zoomOutButton->setMaximumWidth(50);
    zoomLayout->addWidget(zoomOutButton);

    zoomLayout->addStretch(1);

    zoomInButton = new QPushButton("+", this);
    zoomInButton->setMinimumHeight(32);
    zoomInButton->setMaximumWidth(50);
    zoomLayout->addWidget(zoomInButton);

    layout->addLayout(zoomLayout);

    // Berechnungsgenauigkeit Slider
    auto *timeScaleGroup = new QGroupBox("Berechnungsgeschwindigkeit", this);
    auto *timeScaleLayout = new QVBoxLayout(timeScaleGroup);
    
    auto *sliderLayout = new QHBoxLayout();
    auto *slowLabel = new QLabel("Langsam", this);
    sliderLayout->addWidget(slowLabel);
    
    timeScaleSlider = new QSlider(Qt::Horizontal, this);
    timeScaleSlider->setMinimum(1);  // 0.01x
    timeScaleSlider->setMaximum(50);  // 5.0x
    timeScaleSlider->setValue(25);    // 2.5x (Mitte zwischen 0.01 und 5.0 auf log-Skala wäre ~0.7, aber linear Mitte ist 2.55)
    timeScaleSlider->setTickPosition(QSlider::TicksBelow);
    timeScaleSlider->setTickInterval(10);
    sliderLayout->addWidget(timeScaleSlider, 1);
    
    auto *fastLabel = new QLabel("Schnell", this);
    sliderLayout->addWidget(fastLabel);
    
    timeScaleLayout->addLayout(sliderLayout);
    layout->addWidget(timeScaleGroup);

    layout->addStretch(1);

    connect(generateButton, &QPushButton::clicked, this, &MarkerSettingsPanel::emitGenerate);
    connect(toggleAnimationButton, &QPushButton::toggled, this, [this](bool checked) {
        toggleAnimationButton->setText(checked ? "Animation stoppen" : "Animation starten");
        emit animationToggled(checked);
    });
    connect(saveButton, &QPushButton::clicked, this, &MarkerSettingsPanel::saveRequested);
    connect(loadButton, &QPushButton::clicked, this, &MarkerSettingsPanel::loadRequested);
    connect(clearButton, &QPushButton::clicked, this, &MarkerSettingsPanel::clearAllMarkersRequested);
    connect(zoomInButton, &QPushButton::clicked, this, &MarkerSettingsPanel::zoomInRequested);
    connect(zoomOutButton, &QPushButton::clicked, this, &MarkerSettingsPanel::zoomOutRequested);
    connect(timeScaleSlider, &QSlider::valueChanged, this, [this](int value) {
        // Konvertiere von 10-50 zu 0.1-5.0
        float scale = value / 10.0f;
        emit timeScaleChanged(scale);
    });
    
    // Initial time scale setzen
    emit timeScaleChanged(2.5f);
}

void MarkerSettingsPanel::emitGenerate()
{
    bool okCount = false;
    bool okSpeed = false;
    bool okSize = false;
    bool okDensity = false;

    const int count = countEdit->text().toInt(&okCount);
    const float speed = speedEdit->text().toFloat(&okSpeed);
    const float size = sizeEdit->text().toFloat(&okSize);
    const float density = densityEdit->text().toFloat(&okDensity);

    if (!okCount || !okSpeed || !okSize || !okDensity) {
        return;
    }

    emit generateRequested(count, speed, size, density);
}
