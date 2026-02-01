#ifndef MARKERSETTINGSPANEL_H
#define MARKERSETTINGSPANEL_H

#include <QWidget>

class QLineEdit;
class QPushButton;

/**
 * @brief MarkerSettingsPanel - Steuerung fuer Marker-Generierung und Szenarios-Verwaltung
 * 
 * Verantwortlichkeiten:
 * - Eingabeformulare fuer Marker-Generierungsparameter (Anzahl, Geschwindigkeit, Groesse, Dichte)
 * - Steuerknoepfe fuer Animation, Szenarios-Verwaltung (Speichern/Laden) und Zoom
 * - Emission von Signalen bei Benutzerinteraktionen
 * - Verwaltung des Animationsstatus und der UI-Zustandsaenderungen
 */
class MarkerSettingsPanel : public QWidget {
    Q_OBJECT

public:
    explicit MarkerSettingsPanel(QWidget *parent = nullptr);

signals:
    void generateRequested(int count, float speed, float size, float density);
    void animationToggled(bool running);
    void saveRequested();
    void loadRequested();
    void clearAllMarkersRequested();
    void zoomInRequested();
    void zoomOutRequested();

private:
    void emitGenerate();

    QLineEdit *countEdit;
    QLineEdit *speedEdit;
    QLineEdit *sizeEdit;
    QLineEdit *densityEdit;
    QPushButton *generateButton;
    QPushButton *toggleAnimationButton;
    QPushButton *saveButton;
    QPushButton *loadButton;
    QPushButton *clearButton;
    QPushButton *zoomInButton;
    QPushButton *zoomOutButton;
};

#endif // MARKERSETTINGSPANEL_H
