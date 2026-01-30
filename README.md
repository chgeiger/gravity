# Gravity - Qt Application

Eine 3D-Gravitationssimulation auf einer Kugeloberfläche, entwickelt mit Qt6 und Qt3D.

## Über das Projekt

Dies ist eine Programmierübung zur Entwicklung einer interaktiven 3D-Simulation. Das Programm simuliert Objekte (Marker), die sich auf der Oberfläche einer Kugel bewegen und dabei durch Gravitationskräfte miteinander interagieren.

### Features

- **3D-Visualisierung**: Interaktive 3D-Darstellung einer orangefarbenen Kugel mit beweglichen Markern
- **Physik-Simulation**: Gravitations-basierte Interaktion zwischen Markern auf der Kugeloberfläche
- **Marker-Verwaltung**: 
  - Dynamisches Hinzufügen von Markern mit konfigurierbaren Eigenschaften (Geschwindigkeit, Größe)
  - Einzelnes Löschen oder Entfernen aller Marker
  - Visuelle Hervorhebung ausgewählter Marker
- **Kamera-Steuerung**: 
  - Maus-Navigation (Orbit-Kamera)
  - Zoom-Funktionen (In/Out)
- **Objekt-Inspektor**: Baumansicht mit detaillierten Informationen zu jedem Marker
- **Szenario-Verwaltung**: Speichern und Laden von Simulationszuständen
- **Animations-Steuerung**: Start/Stop der Simulation

### Technische Details

- **Sprache**: C++17
- **Framework**: Qt6 (Qt3D, QtWidgets)
- **Build-System**: CMake
- **Architektur**: Model-View-Controller Pattern mit separaten Komponenten für 3D-Rendering, UI und Physik-Simulation

## Anforderungen

- Qt6 (mit Qt3D-Modul)
- CMake 3.16+
- C++17 Compiler
- OpenGL-Unterstützung

## Build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Ausführung

```bash
./Gravity
```

## Bedienung

- **Maus**: Kamera um die Kugel rotieren
- **Mausrad**: Zoom
- **Marker-Tab**: Neue Marker erzeugen und Parameter einstellen
- **Objekte-Tab**: Liste aller Marker mit Details, Anklicken hebt den entsprechenden Marker rot hervor

## Projektstruktur

```
src/
├── main.cpp                 - Einstiegspunkt
├── mainwindow.cpp/h        - Hauptfenster und UI-Verwaltung
├── spherewidget.cpp/h      - 3D-Szene und Physik-Simulation
├── markersettingspanel.cpp/h - Einstellungspanel für Marker
└── surface_marker.cpp/h    - 3D-Marker-Objekt
```

## Lernziele

Dieses Projekt dient als Programmierübung zur Vertiefung von:
- Qt6-Framework und Qt3D für 3D-Grafik
- Physik-Simulation (Gravitation auf gekrümmten Oberflächen)
- C++-Programmierung mit modernen Standards
- MVC-Architektur und Objektorientierung
- CMake-Build-System
- UI-Design mit Qt Widgets
